/***************************************************************************
 *   ls.c - list files and directories                                     *
 *                                                                         *
 *   Copyright (C) 2014 - 2025 by Darren Kirby                             *
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


#include "common.h"

#include <term.h>
#include <curses.h>
#include <dirent.h>
#include <time.h>

const char *APPNAME = "ls";


struct optstruct {
    unsigned int ls_long:1;
    unsigned int human:1;
    unsigned int all:1;
    unsigned int one:1;
    unsigned int inode:1;
    unsigned int dereference:1;
} opts = {0,0,0,0,0,0};


static void show_help(void) {
    printf("Usage: %s [OPTION]... [FILE]...\n\n\
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
    char size_string[22];
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


/*
 * Return a color for a filetype
 */
const char* file_color(mode_t mode)
{
    if (S_ISDIR(mode)) {
        return ANSI_BLUE_B;
    }
    if (S_ISLNK(mode)) {
        return ANSI_CYAN_B;
    }
    if (S_ISSOCK(mode)) {
        return ANSI_MAGENTA;
    }
    if (S_ISFIFO(mode)) {
        return ANSI_YELLOW;
    }
    if (mode & S_IXUSR) {
        return ANSI_GREEN_B;
    }
    return ANSI_RESET;
}


/* Comparison function for strings */
int compare_strings(const void *a, const void *b) {
    return strcmp((const char *)a, (const char *)b);
}


int main(int argc, char *argv[]) {
    int opt;
    int screen_width = 0;

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
            case '?':
                 /*
                  * getopt_long prints own error message
                  *
                  */
                exit(EXIT_FAILURE);
            default:
                show_help();
                exit(EXIT_FAILURE);
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
        strncpy(path_to_ls, ".", 2);
    }

    if ((dp = opendir(path_to_ls)) == NULL){
        perror("opendir");
        exit(EXIT_FAILURE);
    }

    int n_files = 0;          /* number of files to print */
    int n_per_line = 0;       /* number of files per line */
    int longest_so_far = 0;   /* longest filename seen so far */
    int n;                    /* return value of strlen() calls */

    while ((list = readdir(dp)) != NULL) {
        /*
         * first time around
         * get max file length
         */
        if (opts.all == 0) {
            if (list->d_name[0] == '.') {
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

    char filenames[n_files][PATHMAX]; /* dirent strings are 256 bytes */
    n = 0;

    while ((list = readdir(dp)) != NULL) {
        if (opts.all == 0) {

            if (list->d_name[0] == '.') {
                continue;
            }
        }
        strncpy(filenames[n], list->d_name, PATHMAX);
        n++;
    }
    closedir(dp);

    /* sort the filenames alphabetically */
    qsort(filenames, n_files, sizeof(filenames[0]), compare_strings);

    /* cd to path_to_ls */
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

    int f;
    if ((opts.one == 1) && (opts.ls_long != 1)) {
        /*
         * We are displaying short format, one file per line
         */

        for (f = 0; f < n_files; f++) {
            struct stat buf;
            if (lstat(filenames[f], &buf) == -1) {
                perror("lstat");
                exit(EXIT_FAILURE);
            }
            printf("%s%s%s\n", file_color(buf.st_mode), filenames[f], ANSI_RESET);
        }

    } else if (opts.ls_long == 1) {
        /*
         * We are displaying long format, one file per line
         */
        struct stat buf;
        struct tm *now;
        struct tm *fil;
        time_t now_t;
        (void) time(&now_t);
        now = localtime(&now_t);
        int current_year = now->tm_year + 1900;
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
                (void)printf("%6lld ", (long long) buf.st_size) :     /* bytes */
                format((long long)buf.st_size) ;                      /* ie: 16k */

            fil = localtime(&buf.st_mtime);
            if (current_year != (fil->tm_year + 1900)) {
                strftime(string_time, sizeof("Jan 01  1970"), "%b %d  %Y",
                         localtime(&buf.st_mtime));
            } else {
                strftime(string_time, sizeof("Jan 01 12:00"), "%b %d %H:%M",
                         localtime(&buf.st_mtime));
            }

            printf("%s ", string_time);
            printf("%s%s%s\n", file_color(buf.st_mode), filenames[f], ANSI_RESET);
        }

    } else {
        /*
         * We are displaying short format, as many files as we can fit per line
         */
        int i = 1;

        for (f = 0; f < n_files; f++) {
            struct stat buf;
            if (lstat(filenames[f], &buf) == -1) {
                perror("lstat");
                exit(EXIT_FAILURE);
            }

            printf("%s%-*s%s", file_color(buf.st_mode), longest_so_far+1, filenames[f], ANSI_RESET);
            if (i % n_per_line == 0) {
                printf("\n");
            }
            i++;
        }

        if ((i-1) % n_per_line != 0) {
            printf("\n");
        }
    }

    /* Not sure this is even necessary */
    if (chdir(cwd) == -1) {
    perror("chdir");
    exit(EXIT_FAILURE); /* no biggie, already printed the output... */
    }

    return EXIT_SUCCESS;
}
