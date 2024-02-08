/***************************************************************************
 *   vdir.c - print directory contents                                     *
 *                                                                         *
 *   Copyright (C) 2014-2015 by Darren Kirby                               *
 *   bulliver@gmail.com                                                    *
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


#include <sys/types.h>
#include <time.h>
#include <dirent.h>

#include "common.h"
#define APPNAME "vdir"

struct optstruct {
    unsigned int human:1;
    unsigned int all:1;
    unsigned int inode:1;
    unsigned int dereference:1;
    unsigned int colour:1;
    unsigned int classify:1;
} opts;

static void show_help(void) {
    printf("Usage: %s [OPTION]... [FILE]...\n\n\
Options:\n\
    -H, --human\t\tdisplay filesize in kilobytes and megabytes if appropriate (implies --long)\n\
    -a, --all\t\tinclude dotfiles and implied `.' and `..' entries\n\
    -i, --inode\t\tdisplay inode numbers\n\
    -d, --dereference\tshow information for the file links reference rather than for the link itself\n\
    -h, --help\t\tdisplay this help\n\
    -c, --colour\tuse colour output\n\
    -V, --version\tdisplay version information\n\n\
Report bugs to <bulliver@gmail.com>\n", APPNAME);
}

static void p_colour(char * filename, mode_t st_mode) {
    switch (st_mode & S_IFMT) {
        case S_IFBLK:
            printf(ANSI_COLOR_YELLOW "%s" ANSI_COLOR_RESET, filename); // "block device"
            break;
        case S_IFCHR:
            printf(ANSI_COLOR_YELLOW_B "%s" ANSI_COLOR_RESET, filename); // "character device"
            break;
        case S_IFDIR:
            printf(ANSI_COLOR_BLUE_B "%s" ANSI_COLOR_RESET, filename); // "directory"
            break;
        case S_IFIFO:
            printf(ANSI_COLOR_YELLOW "%s" ANSI_COLOR_RESET, filename); // "FIFO/pipe"
            break;
        case S_IFLNK:
            printf(ANSI_COLOR_CYAN_B "%s" ANSI_COLOR_RESET, filename); // "symlink"
            break;
        case S_IFSOCK:
            printf("%s", filename); // "socket"
            break;
        default:   // "regular file"
            /* Is it executable ? */
            if ((st_mode & S_IXUSR) || (st_mode & S_IXGRP) || (st_mode & S_IXOTH)) {
                printf(ANSI_COLOR_GREEN_B "%s" ANSI_COLOR_RESET, filename);
            } else {
                printf("%s", filename);
            }
            break;
    }
}

static void format(long long int bytes) {
    char size_string[10];
    double result;
    if (bytes < 1024) {
        if (sprintf(size_string, "%lld", bytes) < 0) {
            perror("sprintf"); exit(EXIT_FAILURE);
        }
    } else if ((bytes > 1025) && (bytes <= 1025000)) {
        result = bytes / 1024.0;
        if (sprintf(size_string, "%5.1fK", result) < 0) {
            perror("sprintf"); exit(EXIT_FAILURE);
        }
    } else if ((bytes > 1025000) && (bytes <= 1025000000)) {
        result = bytes / 1024.0 / 1024.0;
        if (sprintf(size_string, "%5.1fM", result) < 0) {
            perror("sprintf"); exit(EXIT_FAILURE);
        }
    } else {
        result = bytes / 1024.0 / 1024.0 / 1024.0;
        if (sprintf(size_string, "%5.1fG", result) < 0) {
            perror("sprintf"); exit(EXIT_FAILURE);
        }
    }
    printf("%6s ", size_string);
}

int main(int argc, char *argv[]) {
    int opt;

    struct option longopts[] = {
        {"help", 0, NULL, 'h'},
        {"version", 0, NULL, 'V'},
        {"all", 0, NULL, 'a'},
        {"human", 0, NULL, 'H'},
        {"inode", 0, NULL, 'i'},
        {"dereference", 0, NULL, 'd'},
        {"colour", 0, NULL, 'c'},
        {0,0,0,0}
    };

    while ((opt = getopt_long(argc, argv, "VhaHidc", longopts, NULL)) != -1) {
        switch(opt) {
            case 'V':
                printf("%s (%s) version %s\n", APPNAME, APPSUITE, APPVERSION);
                printf("%s compiled on %s at %s\n", basename(__FILE__), __DATE__, __TIME__);
                exit(EXIT_SUCCESS);
            case 'h':
                show_help();
                exit(EXIT_SUCCESS);
            case 'a':
                opts.all = 1;
                break;
            case 'H':
                opts.human = 1;
                break;
            case 'i':
                opts.inode = 1;
                break;
            case 'd':
                opts.dereference = 1;
                break;
            case 'c':
                opts.colour = 1;
                break;
            case ':':
                 /* getopt_long prints own error message */
                exit(EXIT_FAILURE);
            case '?':
                 /* getopt_long prints own error message */
                exit(EXIT_FAILURE);
            default:
                show_help();
                exit(EXIT_FAILURE);
        }
    }

    DIR *dp;
    struct dirent *list;

    char path_to_ls[PATHMAX];

    if (argv[optind] != NULL) {
        strncpy(path_to_ls, argv[optind], PATHMAX);
    } else {
        strncpy(path_to_ls, ".", 1);
    }

    if ((dp = opendir(path_to_ls)) == NULL){
        perror("opendir");
        exit(EXIT_FAILURE);
    }

    u_int n_files = 0;          /* number of files to print */

    while ((list = readdir(dp)) != NULL) {
        /*
         * first time around
         * get max file length
         */
        if (opts.all == 0) {

            if (strncmp(".",  list->d_name, 1) == 0 ||
                strncmp("..", list->d_name, 2) == 0) {
                continue;
               }
        }

        n_files++;
    }

    rewinddir(dp);

    char filenames[n_files][FILEMAX+1];
    u_int n = 0;

    while ((list = readdir(dp)) != NULL) {
        if (opts.all == 0) {

            if (strncmp(".",  list->d_name, 1) == 0 ||
                strncmp("..", list->d_name, 2) == 0) {
                continue;
               }
        }
        strncpy(filenames[n], list->d_name, FILEMAX+1);
        n++;
    }
    closedir(dp);

    char cwd[PATHMAX];
    char *cwd_p;
    cwd_p = cwd;

    if (getcwd(cwd_p, PATHMAX) == NULL) {
        perror("getcwd");
        exit(EXIT_FAILURE);
    }

    if (chdir(path_to_ls) == -1) {
        perror("chdir");
        exit(EXIT_FAILURE);
    }

    struct stat buf;
    struct tm *now;
    struct tm *fil;
    time_t now_t;
    (void) time(&now_t);
    now = localtime(&now_t);
    u_int current_year = now->tm_year + 1900;
    char string_time[13];

    for (int f = 0; f < n_files; f++) {
        if (opts.dereference == 1) {
            if (stat(filenames[f], &buf) == -1) {
                perror("stat");
                exit(EXIT_FAILURE);
            }
        } else {
            if (lstat(filenames[f], &buf) == -1) {
                perror("stat");
                exit(EXIT_FAILURE);
            }
        }

        if (opts.inode == 1) {
            printf("%8d ", (int) buf.st_ino);
        }
        printf("%s", filetype(buf.st_mode, 0));
        printf("%s ", file_perm_str(buf.st_mode, 1));
        printf("%2ld ", (long) buf.st_nlink);
        printf("%s %s ", get_username(buf.st_uid), get_groupname(buf.st_gid));
        (opts.human == 0) ?
            printf("%6lld ", (long long) buf.st_size) :       /* bytes */
            format((long long)buf.st_size) ;                  /* ie: 16k */

        fil = localtime(&buf.st_mtime);
        if (current_year != (fil->tm_year + 1900)) {
            strftime(string_time, sizeof("Jan 01  1970"), "%b %d  %Y", localtime(&buf.st_mtime));
        } else {
            strftime(string_time, sizeof("Jan 01 12:00"), "%b %d %H:%M", localtime(&buf.st_mtime));
        }

        printf("%s ", string_time);

        if (opts.colour == 1) {
            p_colour(filenames[f], buf.st_mode);
        } else {
            printf("%s", filenames[f]);
        }
        printf("\n");
    }

    if (chdir(cwd) == -1) {
        perror("chdir");
        exit(EXIT_FAILURE); /* no biggie, already printed the output... */
    }

    return EXIT_SUCCESS;
}
