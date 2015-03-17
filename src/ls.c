/***************************************************************************
 *   ls.c - list files and directories                                     *
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


#define APPNAME "ls"
#include "common.h"

#include <term.h>
#include <curses.h>
#include <dirent.h>

struct optstruct {
    unsigned int ls_long:1;
    unsigned int human:1;
    unsigned int all:1;
    unsigned int one:1;
    unsigned int inode:1;
    unsigned int dereference:1;
} opts;


static void show_help(void) {
    printf("Usage: %s [OPTION]...\n\n\
Options:\n\
    -l, --long\t\toutput long format listing\n\
    -H, --human-readable\t\tdisplay filesize in kilobytes and megabytes if appropriate (implies --long)\n\
    -a, --all\t\tinclude dotfiles and implied `.' and `..' entries\n\
    -1, --one\t\tlist files one per line\n\
    -i, --inode\t\tdisplay inode numbers (implies --long)\n\
    -d, --dereference\tshow information for the file links reference rather than for the link itself\n\
    -h, --help\t\tdisplay this help\n\
    -V, --version\tdisplay version information\n\n\
Report bugs to <bulliver@gmail.com>\n", APPNAME);
}

void format(long long int bytes) {
    char size_string[10];
    double result;
    if (bytes < 1024) {
        sprintf(size_string, "%lld", bytes);
    } else if ((bytes > 1025) && (bytes <= 1025000)) {
        result = bytes / 1024.0;
        sprintf(size_string, "%5.1fK", result);
    } else if ((bytes > 1025000) && (bytes <= 1025000000)) {
        result = bytes / 1024.0 / 1024.0;
        sprintf(size_string, "%5.1fM", result);
    }
    printf("%s ", size_string);
}

int main(int argc, char *argv[]) {
    int opt;
    opts.all = 0;

    struct option longopts[] = {
        {"help", 0, NULL, 'h'},
        {"version", 0, NULL, 'V'},
        {"all", 0, NULL, 'a'},
        {"human", 0, NULL, 'H'},
        {"long", 0, NULL, 'l'},
        {"one", 0, NULL, '1'},
        {"inode", 0, NULL, 'i'},
        {"dereference", 0, NULL, 'd'},
        {0,0,0,0}
    };


    while ((opt = getopt_long(argc, argv, "VhalH1id", longopts, NULL)) != -1) {
        switch(opt) {
            case 'V':
                printf("%s (%s) version %s\n", APPNAME, APPSUITE, APPVERSION);
                printf("%s compiled on %s at %s\n", basename(__FILE__), __DATE__, __TIME__);
                exit(EXIT_SUCCESS);
                break;
            case 'h':
                show_help();
                exit(EXIT_SUCCESS);
            case 'a':
                opts.all = 1;
                break;
            case 'l':
                opts.ls_long = 1;
                opts.one = 1;     /* '-l' implies '-1' one */
                break;
            case '1':
                opts.one = 1;
                break;
            case 'i':
                opts.inode = 1;
                break;
            case 'd':
                opts.dereference = 1;
                break;
            case 'H':
                opts.human = 1;
                opts.ls_long = 1; /* '-H' implies '-l' ls_long */
                opts.one = 1;     /* '-H' implies '-1' one     */
                break;
            case ':':
                 /* getopt_long prints own error message */
                exit(EXIT_FAILURE);
                break;
            case '?':
                 /* getopt_long prints own error message */
                exit(EXIT_FAILURE);
            default:
                show_help();
                exit(EXIT_FAILURE);
                break;
        }
    }

    printf("filemax: %d\n", filemax);
    printf("pathmax: %d\n", pathmax);

    /* get width of terminal */
    setupterm(NULL, fileno(stdout), (int *)0);
    int work_col = tigetnum("cols") - 30;
    int n_per_line = 0;
    int longest_so_far = 0;
    int n;

    DIR *dp;
    struct dirent *list;

    char path_to_ls[pathmax];

    if (argv[optind] != NULL) {
        strncpy(path_to_ls, argv[optind], pathmax);
    } else {
        strncpy(path_to_ls, ".", 1);
    }

    if ((dp = opendir(path_to_ls)) == NULL){
        perror("opendir");
    }

    int n_files = 0; /* num files */

    while ((list = readdir(dp)) != NULL) {
        /* first time around   */
        /* get max file length */
        if (opts.all == 0) {

            if (strncmp(".",  list->d_name, 1) == 0 ||
                strncmp("..", list->d_name, 2) == 0) {
                continue;
               }
        }

        n_files++;

        if ((n = strlen(list->d_name)) > longest_so_far) {
            longest_so_far = n;
        }
    }

    n_per_line = work_col / (longest_so_far+2); /* number of filenames per column */
    rewinddir(dp);

    char filenames[n_files][filemax+1];
    n = 0;

    while ((list = readdir(dp)) != NULL) {
        if (opts.all == 0) {

            if (strncmp(".",  list->d_name, 1) == 0 ||
                strncmp("..", list->d_name, 2) == 0) {
                continue;
               }
        }
        strncpy(filenames[n], list->d_name, filemax+1);
        n++;
    }
    closedir(dp);

    int f;

    if ((opts.one == 1) && (opts.ls_long != 1)) {
        for (int f = 0; f < n_files; f++) {
            printf("%s\n", filenames[f]);
        }
    } else if (opts.ls_long == 1) {
        char cwd[pathmax];
        char *cwd_p;
        cwd_p = cwd;
        char string_time[13];

        getcwd(cwd_p, pathmax);
        chdir(path_to_ls);

        struct stat buf;
        struct tm *now;
        struct tm *fil;
        time_t now_t;
        (void) time(&now_t);
        now = localtime(&now_t);
        int current_year = now->tm_year + 1900;

        for (f = 0; f < n_files; f++) {
            if (opts.dereference == 1) {
                if (stat(filenames[f], &buf) == -1) {
                    perror("stat");
                }
            } else {
                if (lstat(filenames[f], &buf) == -1) {
                    perror("stat");
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
            printf("%s", filenames[f]);
            printf("\n");
        }
        chdir(cwd);

    } else {
        int i = 1;

        for (f = 0; f < n_files; f++) {
            printf("%-*s", longest_so_far+2, filenames[f]);
            if (i % n_per_line == 0) {
                printf("\n");
            }
            i++;
        }
        if ((i-1) % n_per_line != 0) {
            printf("\n");
        }
    }

    return EXIT_SUCCESS;
}
