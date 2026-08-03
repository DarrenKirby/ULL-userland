/***************************************************************************
 *   ls.c - list files and directories                                     *
 *                                                                         *
 *   Copyright (C) 2014 - 2026 by Darren Kirby                             *
 *   darren@dragonbyte.ca                                                  *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/

#include <sys/ioctl.h>
#include <dirent.h>
#include <time.h>


#include "ls.h"

#define DIRENT_STRING_SIZE 256

char *APP_NAME = "ls";

struct Opts opts = {
    .fields = 0,
    .screen_width = 0,
    .time = 0,
    .ls_long = false,
    .human = false,
    .all = false,
    .one = false,
    .dereference = false,
    .colour = true,
    .classify = false,
    .size = false,
    .reverse = false };


int main(const int argc, char *argv[])
{
    if (process_args(argc, argv) != 0) {
        return EXIT_FAILURE;
    }

    /* Get width of term. */
    if (opts.screen_width == 0) {
        opts.screen_width = get_screen_width();
    }

    const size_t file_max = get_filename_max();
    char path_to_ls[file_max];

    const int n_args = argc - optind;
    bool no_arg = n_args == 0 ? true : false;

    do {
        if (argv[optind] != NULL) {
            strlcpy(path_to_ls, argv[optind], sizeof(path_to_ls));
            no_arg = false;
        } else {
            /* If there were args, but we get here, it means
             * the args have all been iterated over. Just break. */
            if (no_arg) {
                strlcpy(path_to_ls, ".", sizeof(path_to_ls));
            } else {
                break;
            }
        }

        bool arg_is_dir;

        DIR *dp;
        struct dirent *list;
        errno = 0;
        if ((dp = opendir(path_to_ls)) == NULL) {
            if (errno == ENOTDIR) {
                arg_is_dir = false;
            } else {
                fprintf(stderr, "%s: opendir failed: %s", APP_NAME, strerror(errno));
                return EXIT_FAILURE;
            }
        } else {
            arg_is_dir = true;
        }

        int32_t n_files = 0;           /* Number of files to print. */
        uint32_t longest_so_far = 0;   /* Longest filename seen so far. */

        if (arg_is_dir) {
            uint32_t n;
            while ((list = readdir(dp)) != NULL) {
                /* First time around, get max file length. */
                if (!opts.all) {
                    if (list->d_name[0] == '.') {
                        continue;
                    }
                }
                n_files++;

                if ((n = strlen(list->d_name)) > longest_so_far) {
                    longest_so_far = n;
                }
            }
            rewinddir(dp);

            char* filenames[n_files];
            n = 0;

            while ((list = readdir(dp)) != NULL) {
                if (!opts.all) {

                    if (list->d_name[0] == '.') {
                        continue;
                    }
                }
                filenames[n] = strdup(list->d_name);
                n++;
            }
            closedir(dp);

            /* `cd` to path_to_ls. This needs to be done before the
             * qsort, as compare_size and compare_time calls stat. */
            char cwd[file_max];
            char *cwd_p = cwd;

            if (getcwd(cwd_p, file_max) == NULL) {
                fprintf(stderr, "%s: getcwd() failed: %s\n", APP_NAME, strerror(errno));
                return EXIT_FAILURE;
            }

            if (chdir(path_to_ls) == -1) {
                fprintf(stderr, "%s: chdir failed: %s", APP_NAME, strerror(errno));
                return EXIT_FAILURE;
            }

            /* Sort the filenames as per options. */
            if (opts.size) {
                qsort(filenames, n_files, sizeof(char*), compare_size);
            } else if (opts.time) {
                switch (opts.time) {
                    case 1: qsort(filenames, n_files, sizeof(char*), compare_atime); break;
                    case 2: qsort(filenames, n_files, sizeof(char*), compare_mtime); break;
                    case 3: qsort(filenames, n_files, sizeof(char*), compare_ctime); break;
                    default: break;
                }
            } else {
                /* Default alphabetic sort. */
                qsort(filenames, n_files, sizeof(char*), compare_strings);
            }

            /* Reverse the array to print ascending order. */
            if (opts.reverse) {
                reverseArray(filenames, n_files);
                //reverse_array(n_files, file_max, filenames);
            }

            if (n_args > 1) {
                printf("\n%s:\n", path_to_ls);
            }

            if (opts.one && !opts.ls_long) {
                print_one_format(n_files, filenames);
            } else if (opts.ls_long) {
                print_long_format(n_files, filenames);
            } else {
                print_short_format(n_files, filenames, longest_so_far);
            }

            if (chdir(cwd_p) == -1) {
                fprintf(stderr, "%s: chdir failed: %s", APP_NAME, strerror(errno));
                return EXIT_FAILURE;
            }

            /* Deallocate the duped strings. */
            for (int i = 0; i < n_files; i++) {
                free(filenames[i]);
            }
        } else {
            /* Just a single file... */
            char *filenames[1];
            filenames[0] = strdup(path_to_ls);
            //memcpy(filenames, path_to_ls, strlen(path_to_ls) + 1);

            if (opts.one && !opts.ls_long) {
                print_one_format(n_files, filenames);
            } else if (opts.ls_long) {
                print_long_format(n_files, filenames);
            } else {
                print_short_format(n_files, filenames, longest_so_far);
            }
            free(filenames[0]);
        }

    } while (argc > optind++);

    return EXIT_SUCCESS;
}
