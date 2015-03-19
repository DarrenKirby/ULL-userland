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

struct filestruct {
    char         filename[PATHMAX];
    u_int        type;
    ssize_t      size;
} file_s;

static void show_help(void) {
    printf("Usage: %s [OPTION]...\n\n\
Options:\n\
    -l, --long\t\toutput long format listing\n\
    -H, --human\t\tdisplay filesize in kilobytes and megabytes if appropriate (implies --long)\n\
    -a, --all\t\tinclude dotfiles and implied `.' and `..' entries\n\
    -1, --one\t\tlist files one per line\n\
    -i, --inode\t\tdisplay inode numbers (implies --long)\n\
    -d, --dereference\tshow information for the file links reference rather than for the link itself\n\
    -h, --help\t\tdisplay this help\n\
    -V, --version\tdisplay version information\n\n\
Report bugs to <bulliver@gmail.com>\n", APPNAME);
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
    printf("%s ", size_string);
}


int main(int argc, char *argv[]) {
    int opt;
    opts.all = 0;
    u_int screen_width = 0;

    struct option longopts[] = {
        {"help", 0, NULL, 'h'},
        {"version", 0, NULL, 'V'},
        {"all", 0, NULL, 'a'},
        {"human", 0, NULL, 'H'},
        {"long", 0, NULL, 'l'},
        {"one", 0, NULL, '1'},
        {"inode", 0, NULL, 'i'},
        {"dereference", 0, NULL, 'd'},
        {"width", required_argument, NULL, 'w'},
        {0,0,0,0}
    };

    while ((opt = getopt_long(argc, argv, "VhalH1idw:", longopts, NULL)) != -1) {
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
            case 'w':
                screen_width = (int)strtol(optarg, NULL, 10);
                break;
            case 'H':
                opts.human = 1;
                opts.ls_long = 1; /* '-H' implies '-l' ls_long */
                opts.one = 1;     /* '-H' implies '-1' one     */
                break;
            case ':':
                 /*
                  * getopt_long prints own error message
                  *
                  */
                exit(EXIT_FAILURE);
                break;
            case '?':
                 /*
                  * getopt_long prints own error message
                  *
                  */
                exit(EXIT_FAILURE);
            default:
                show_help();
                exit(EXIT_FAILURE);
                break;
        }
    }

    /*
     * get width of terminal
     */
    if (screen_width == 0) {
        setupterm(NULL, fileno(stdout), (int *)0);
        screen_width = tigetnum("cols");
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
    u_int n_per_line = 0;       /* number of files per line */
    u_int longest_so_far = 0;   /* longest filename seen so far */
    int n;                      /* return value of strlen() calls */

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

        if ((n = strlen(list->d_name)) > longest_so_far) {
            longest_so_far = n;
        }
    }

    n_per_line = screen_width / (longest_so_far+2); /* number of filenames per column */
    rewinddir(dp);

    char filenames[n_files][FILEMAX+1];
    n = 0;

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

    u_int f;

    if ((opts.one == 1) && (opts.ls_long != 1)) {
        /*
         * We are displaying short format, one file per line
         */
        for (int f = 0; f < n_files; f++) {
            printf("%s\n", filenames[f]);
        }

    } else if (opts.ls_long == 1) {
        /*
         * We are displaying long format, one file per line
         */

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

        for (f = 0; f < n_files; f++) {
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
            printf("%s", filenames[f]);
            printf("\n");
        }

        if (chdir(cwd) == -1) {
            perror("chdir");
            exit(EXIT_FAILURE); /* no biggie, already printed the output... */
        }

    } else {
        /*
         * We are displaying short format, as many files as we can fit per line
         */
        u_int i = 1;

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
