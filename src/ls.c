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
#include <getopt.h>
#include <inttypes.h>

#include "common.h"

#define DIRENT_STRING_SIZE 256

static const char *APP_NAME = "ls";

static struct {
    bool ls_long:1;
    bool human:1;
    bool all:1;
    bool one:1;
    bool inode:1;
    bool dereference:1;
} opts = { .ls_long = false,
           .human = false,
           .all = false,
           .one = false,
           .inode = false,
           .dereference = false };


static void show_help()
{
    printf("Usage: %s [OPTION]... [FILE]...\n\n\
List and show info for files and directories\n\n\
Options:\n\
    -l, --long\t\toutput long format listing\n\
    -H, --human\t\tdisplay file size in kilobytes and megabytes if appropriate (implies --long)\n\
    -a, --all\t\tinclude dotfiles and implied `.' and `..' entries\n\
    -1, --one\t\tlist files one per line\n\
    -i, --inode\t\tdisplay inode numbers (implies --long)\n\
    -d, --dereference\tshow information for the file links reference rather than for the link itself\n\
    -w, --width=N\tset display screen width to 'N' characters\n\
    -h, --help\t\tdisplay this help\n\
    -V, --version\tdisplay version information\n\n\
Report bugs to <darren@dragonbyte.ca>\n", APP_NAME);
}

/* Return a color for a filetype. */
static const char* file_color(const mode_t mode)
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

/* Comparison function for strings. */
static int compare_strings(const void *a, const void *b)
{
    return strcmp(a, b);
}

int main(const int argc, char *argv[])
{
    uint16_t screen_width = 0;
    const struct option long_opts[] = {
        { .name = "help",        .has_arg = no_argument,       .flag = nullptr, .val = 'h' },
        { .name = "version",     .has_arg = no_argument,       .flag = nullptr, .val = 'V' },
        { .name = "all",         .has_arg = no_argument,       .flag = nullptr, .val = 'a' },
        { .name = "human",       .has_arg = no_argument,       .flag = nullptr, .val = 'H' },
        { .name = "long",        .has_arg = no_argument,       .flag = nullptr, .val = 'l' },
        { .name = "one",         .has_arg = no_argument,       .flag = nullptr, .val = '1' },
        { .name = "inode",       .has_arg = no_argument,       .flag = nullptr, .val = 'i' },
        { .name = "dereference", .has_arg = no_argument,       .flag = nullptr, .val = 'd' },
        { .name = "width",       .has_arg = required_argument, .flag = nullptr, .val = 'w' },
        { .name = nullptr,       .has_arg = no_argument,       .flag = nullptr, .val = 0 }
    };

    int opt;
    while ((opt = getopt_long(argc, argv, "VhalH1idw:", long_opts, nullptr)) != -1) {
        switch(opt) {
            case 'V':
                printf("%s (%s) version %s\n", APP_NAME, APP_SUITE, APP_VERSION);
                printf("%s compiled on %s at %s\n",
                       strrchr(__FILE__, '/') ? strrchr(__FILE__, '/') + 1 : __FILE__,
                       __DATE__, __TIME__);
                return EXIT_SUCCESS;
            case 'h':
                show_help();
                return EXIT_SUCCESS;
            case 'a':
                opts.all = true;
                break;
            case 'l':
                opts.ls_long = true;
                opts.one = true;     /* '-l' implies '-1' one */
                break;
            case '1':
                opts.one = true;
                break;
            case 'i':
                opts.inode = true;
                opts.ls_long = true;   /* '-i' implies '-l' */
                break;
            case 'd':
                opts.dereference = true;
                break;
            case 'w':
                screen_width = (uint16_t)strtoul(optarg, nullptr, 10);
                break;
            case 'H':
                opts.human = true;
                opts.ls_long = true;   /* '-H' implies '-l' ls_long */
                opts.one = true;       /* '-H' implies '-1' one     */
                break;
            default:
                show_help();
                return EXIT_FAILURE;
        }
    }

    /* Get width of term. */
    if (screen_width == 0) {
        /* Check if stdout is redirected to a file or a pipe. */
        if (!isatty(STDOUT_FILENO)) {
            /* Just set a reasonable value. */
            screen_width = 82;
        } else {
            struct winsize w;
            if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &w) != 0) {
                fprintf(stderr, "ioctl failed: %s\n",
                    strerror(errno));
                return EXIT_FAILURE;
            }
            screen_width = w.ws_col;
        }
    }

    const size_t path_max = get_path_max();
    char path_to_ls[path_max];

    if (argv[optind] != NULL) {
        strlcpy(path_to_ls, argv[optind], sizeof(path_to_ls));
    } else {
        strlcpy(path_to_ls, ".", sizeof(path_to_ls));
    }

    DIR *dp;
    struct dirent *list;
    if ((dp = opendir(path_to_ls)) == NULL) {
        fprintf(stderr, "%s: opendir failed: %s", APP_NAME, strerror(errno));
        return EXIT_FAILURE;
    }

    int32_t n_files = 0;           /* Number of files to print. */
    uint32_t longest_so_far = 0;   /* Longest filename seen so far. */
    uint32_t n;                    /* Return value of strlen() calls. */

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

    char filenames[n_files][path_max];
    n = 0;

    while ((list = readdir(dp)) != NULL) {
        if (!opts.all) {

            if (list->d_name[0] == '.') {
                continue;
            }
        }
        snprintf(filenames[n], sizeof(filenames[n]), "%s", list->d_name);
        n++;
    }
    closedir(dp);

    /* Sort the filenames alphabetically. */
    qsort(filenames, n_files, sizeof(filenames[0]), compare_strings);

    /* `cd` to path_to_ls. */
    char cwd[path_max];
    char *cwd_p;
    cwd_p = cwd;

    if (getcwd(cwd_p, path_max) == NULL) {
        fprintf(stderr, "%s: getcwd() failed: %s\n", APP_NAME, strerror(errno));
        return EXIT_FAILURE;
    }

    if (chdir(path_to_ls) == -1) {
        fprintf(stderr, "%s: chdir failed: %s", APP_NAME, strerror(errno));
        return EXIT_FAILURE;
    }

    int f;
    if (opts.one && (!opts.ls_long)) {
        /* We are displaying short format, one file per line. */
        for (f = 0; f < n_files; f++) {
            struct stat buf;
            if (lstat(filenames[f], &buf) == -1) {
                fprintf(stderr, "%s: lstat failed: %s", APP_NAME, strerror(errno));
                return EXIT_FAILURE;
            }
            printf("%s%s%s\n", file_color(buf.st_mode), filenames[f], ANSI_RESET);
        }

    } else if (opts.ls_long) {
        /*  We are displaying long format, one file per line. */
        struct stat buf;
        struct tm *now;
        struct tm *fil;
        time_t now_t;
        (void) time(&now_t);
        now = localtime(&now_t);
        int current_year = now->tm_year + 1900;
        char string_time[13];

        for (f = 0; f < n_files; f++) {
            if (opts.dereference) {
                if (stat(filenames[f], &buf) == -1) {
                    fprintf(stderr, "%s: %s", filenames[f], strerror(errno));
                    return EXIT_FAILURE;
                }
            } else {
                if (lstat(filenames[f], &buf) == -1) {
                    fprintf(stderr, "%s: %s", filenames[f], strerror(errno));
                    return EXIT_FAILURE;
                }
            }

            if (opts.inode) {
                printf("%8d ", (int) buf.st_ino);
            }
            printf("%s", filetype(buf.st_mode, 0));
            printf("%s ", file_perm_str(buf.st_mode, 1));
            printf("%2ld ", (long) buf.st_nlink);
            printf("%s %s ", get_username(buf.st_uid), get_groupname(buf.st_gid));
            !opts.human ? (void)printf("%6" PRId64 " ", buf.st_size) :
                format(buf.st_size);    /* ie: 16k */

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
        /* Short-format. As many as can fit per line. */
        constexpr int padding = 2;

        /* The absolute maximum number of columns is if every file was 1 char long */
        int max_cols = screen_width / (1 + padding);
        if (max_cols > n_files) {
            max_cols = n_files;
        }
        if (max_cols < 1) {
            max_cols = 1;
        }

        int valid_cols = 1;
        int rows = n_files;
        int col_widths[max_cols];

        /* Brute-force guess the columns from maximum down to 1. */
        for (int c = max_cols; c > 1; c--) {
            int r = (n_files + c - 1) / c;
            int total_width = 0;

            /* Calculate the max width required for each specific column. */
            for (int i = 0; i < c; i++) {
                col_widths[i] = 0;

                for (int j = 0; j < r; j++) {
                    int index = (i * r) + j;
                    if (index < n_files) {
                        int len = strlen(filenames[index]);
                        if (len > col_widths[i]) {
                            col_widths[i] = len;
                        }
                    }
                }

                total_width += col_widths[i];
                if (i < c - 1) {
                    total_width += padding;
                }
            }

            /* If this configuration fits the terminal, we have a winner. */
            if (total_width <= screen_width) {
                valid_cols = c;
                rows = r;
                break;
            }
        }

        /* If we dropped all the way to 1 column, set its width to the longest file. */
        if (valid_cols == 1) {
            col_widths[0] = longest_so_far;
        }

        for (int r = 0; r < rows; r++) {
            for (int c = 0; c < valid_cols; c++) {
                int index = c * rows + r;

                if (index < n_files) {
                    struct stat buf;
                    if (lstat(filenames[index], &buf) == -1) {
                        fprintf(stderr, "%s: lstat() failed on %s: %s", APP_NAME,
                            filenames[index], strerror(errno));
                        return EXIT_FAILURE;
                    }

                    /* If it's the last column, don't pad the right side. */
                    if (c == valid_cols - 1) {
                        printf("%s%s%s", file_color(buf.st_mode), filenames[index], ANSI_RESET);
                    } else {
                        printf("%s%-*s%s", file_color(buf.st_mode), col_widths[c] + padding,
                            filenames[index], ANSI_RESET);
                    }
                }
            }
            printf("\n");
        }
    }

    return EXIT_SUCCESS;
}
