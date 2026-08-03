/***************************************************************************
 *   ls.h - includes and functions common to ls, dir, and vdir             *
 *                                                                         *
 *   Copyright (C) 2014-2026 by Darren Kirby                               *
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

#ifndef LS_H
#define LS_H

#include <getopt.h>
#include <inttypes.h>

#include "common.h"

#define PATH_MAX 4096

#define OPT_ATIME 256
#define OPT_MTIME 257
#define OPT_CTIME 258


extern char* APP_NAME;

extern inline void show_help()
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

struct Opts {
    uint16_t fields;
    uint16_t screen_width;
    uint16_t time;
    bool ls_long:1;        /* Print in long-format. */
    bool human:1;          /* Print human-readable file sizes. */
    bool all:1;            /* Include dot files. */
    bool one:1;            /* Print one file per line. */
    bool dereference:1;    /* Dereference symlinks. */
    bool colour:1;         /* Print files/dirs in colour. */
    bool classify:1;       /* Append filenames with classifiers. */
    bool size:1;           /* Sort by size in bytes. */
    bool reverse:1;        /* Print files in ascending order. */
};

extern struct Opts opts;

extern inline int process_args(const int argc, char** argv)
{
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
         { .name = "classify",    .has_arg = no_argument,       .flag = nullptr, .val = 'F' },
         { .name = "no-colour",   .has_arg = no_argument,       .flag = nullptr, .val = 'n' },
         { .name = "colour",      .has_arg = no_argument,       .flag = nullptr, .val = 'c' },
         { .name = "size",        .has_arg = no_argument,       .flag = nullptr, .val = 's' },
         { .name = "reverse",     .has_arg = no_argument,       .flag = nullptr, .val = 'r' },
         { .name = "atime",       .has_arg = no_argument,       .flag = nullptr, .val = OPT_ATIME },
         { .name = "mtime",       .has_arg = no_argument,       .flag = nullptr, .val = OPT_MTIME },
         { .name = "ctime",       .has_arg = no_argument,       .flag = nullptr, .val = OPT_CTIME },
         { .name = nullptr,       .has_arg = no_argument,       .flag = nullptr, .val = 0 }
    };

    int opt;
    while ((opt = getopt_long(argc, argv, "VhalH1idw:Fncsr", long_opts, nullptr)) != -1) {
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
                // FIXME opts.inode = true;
                opts.ls_long = true;   /* '-i' implies '-l' */
                break;
            case 'd':
                opts.dereference = true;
                break;
            case 'w':
                opts.screen_width = (uint16_t)strtoul(optarg, nullptr, 10);
                break;
            case 'H':
                opts.human = true;
                opts.ls_long = true;   /* '-H' implies '-l' ls_long */
                opts.one = true;       /* '-H' implies '-1' one     */
                break;
            case 'c':
                opts.colour = true;
                break;
            case 'n':
                opts.colour = false;
                break;
            case 'F':
                opts.classify = true;
                break;
            case 's':
                opts.size = true;
                opts.time = 0;
                break;
            case 'r':
                opts.reverse = true;
                break;
            case OPT_ATIME:
                opts.time = 1;
                break;
            case OPT_MTIME:
                opts.time = 2;
                break;
            case OPT_CTIME:
                opts.time = 3;
                break;
            default:
                show_help();
                return EXIT_FAILURE;
        }
    }
    return EXIT_SUCCESS;
}

extern inline uint16_t get_screen_width() {
    uint16_t screen_width;
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
    return screen_width;
}


extern inline void resolve_link(const char *path, char buf[]) {
    const ssize_t len = readlink(path, buf, PATH_MAX);
    if (len == -1) {
        memset(buf, '?', 3);
        buf[3] = '\0';
        return;
    }
    buf[len] = '\0';
}

extern inline int p_filename(char *filename, const struct stat buf,
                                   const bool colour, const bool classify)
{
    int len = strlen(filename);
    switch (buf.st_mode & S_IFMT) {
        /* block device */
        case S_IFBLK:
            printf("%s%s%s", colour ? ANSI_YELLOW : "", filename,
                               colour ? ANSI_RESET : "");
            break;
        /* character device */
        case S_IFCHR:
            printf("%s%s%s", colour ? ANSI_YELLOW_B : "", filename,
                               colour ? ANSI_RESET : "");
            break;
        /* directory */
        case S_IFDIR:
            if (classify) len++;
            printf("%s%s%s%s", colour ? ANSI_BLUE_B : "", filename,
                               colour ? ANSI_RESET : "",  classify ? "/" : "");
            break;
        /* FIFO/pipe */
        case S_IFIFO:
            if (classify) len++;
            printf("%s%s%s%s", colour ? ANSI_YELLOW : "", filename,
                               colour ? ANSI_RESET : "",  classify ? "|" : "");
            break;
        /* symlink */
        case S_IFLNK: {
            if (opts.ls_long) {
            char l_buf[PATH_MAX];
            resolve_link(filename, l_buf);
            printf("%s%s%s -> %s", colour ? ANSI_CYAN_B : "", filename,
                                   colour ? ANSI_RESET : "", l_buf);
            } else {
                if (classify) len++;
                printf("%s%s%s%s", colour ? ANSI_CYAN_B : "", filename,
                                   colour ? ANSI_RESET : "", classify ? "@" : "");
            }
            break;
        }
        /* socket */
        case S_IFSOCK:
            if (classify) len++;
            printf("%s%s%s%s", colour ? ANSI_MAGENTA_B : "", filename,
                               colour ? ANSI_RESET    : "",  classify ? "=" : "");
            break;
        /* regular file */
        default:
            /* Is it executable ? */
            if (buf.st_mode & S_IXUSR || buf.st_mode & S_IXGRP || buf.st_mode & S_IXOTH) {
                if (classify) len++;
                printf("%s%s%s%s", colour ? ANSI_GREEN_B : "", filename,
                                   colour ? ANSI_RESET : "",  classify ? "*" : "");
            } else {
                printf("%s", filename);
            }
            break;
    }
    return len;
}

extern inline void print_one_format(const int32_t n_files, char* filenames[]) {
    /* We are displaying short format, one file per line. */
    for (int f = 0; f < n_files; f++) {
        struct stat buf;
        if (lstat(filenames[f], &buf) == -1) {
            fprintf(stderr, "%s: lstat failed: %s", APP_NAME, strerror(errno));
            exit(EXIT_FAILURE);
        }

        p_filename(filenames[f], buf, opts.colour, opts.classify);
        putchar('\n');
    }
}

extern inline void print_long_format(const int32_t n_files, char *filenames[])
{
    /*  We are displaying long format, one file per line. */
    struct stat buf;
    time_t now_t;
    (void) time(&now_t);
    const struct tm *now = localtime(&now_t);
    const int current_year = now->tm_year + 1900;
    char string_time[13];

    for (int f = 0; f < n_files; f++) {
        if (opts.dereference) {
            if (stat(filenames[f], &buf) == -1) {
                fprintf(stderr, "%s: %s", filenames[f], strerror(errno));
                exit(EXIT_FAILURE);
            }
        } else {
            if (lstat(filenames[f], &buf) == -1) {
                fprintf(stderr, "%s: %s", filenames[f], strerror(errno));
                exit(EXIT_FAILURE);
            }
        }

        // if (opts.inode) {
        //     printf("%8d ", (int) buf.st_ino);
        // }
        printf("%s", filetype(buf.st_mode, 0));
        printf("%s ", file_perm_str(buf.st_mode, 1));
        printf("%2ld ", (long) buf.st_nlink);
        printf("%s %s ", get_username(buf.st_uid), get_groupname(buf.st_gid));
        !opts.human ? (void)printf("%6" PRId64 " ", buf.st_size) :
            format_ls(buf.st_size);    /* ie: 16k */

        const struct tm *fil = localtime(&buf.st_mtime);
        if (current_year != fil->tm_year + 1900) {
            strftime(string_time, sizeof("Jan 01  1970"), "%b %d  %Y",
                     localtime(&buf.st_mtime));
        } else {
            strftime(string_time, sizeof("Jan 01 12:00"), "%b %d %H:%M",
                     localtime(&buf.st_mtime));
        }

        printf("%s ", string_time);
        p_filename(filenames[f], buf, opts.colour, opts.classify);
        putchar('\n');
    }
}

extern inline void print_short_format(const int32_t n_files, char *filenames[],
    const uint32_t longest_so_far)
{
    /* Short-format. As many as can fit per line. */
    constexpr int padding = 2;

    /* The absolute maximum number of columns is if every file was 1 char long */
    int max_cols = opts.screen_width / (1 + padding);
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
        const int r = (n_files + c - 1) / c;
        int total_width = 0;

        /* Calculate the max width required for each specific column. */
        for (int i = 0; i < c; i++) {
            col_widths[i] = 0;

            for (int j = 0; j < r; j++) {
                const int index = i * r + j;
                if (index < n_files) {
                    int len = strlen(filenames[index]);

                    /* If classify is on, assume the worst-case (+1 char)
                       so we don't have to lstat() every file here. */
                    if (opts.classify) {
                        len++;
                    }

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
        if (total_width <= opts.screen_width) {
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
            const int index = c * rows + r;

            if (index < n_files) {
                struct stat buf;
                if (lstat(filenames[index], &buf) == -1) {
                    fprintf(stderr, "%s: lstat() failed on %s: %s", APP_NAME,
                        filenames[index], strerror(errno));
                    exit(EXIT_FAILURE);
                }

                /* If it's the last column, don't pad the right side. */
                if (c == valid_cols - 1) {
                    p_filename(filenames[index], buf, opts.colour, opts.classify);
                } else {
                    const int printed_len = p_filename(filenames[index], buf, opts.colour, opts.classify);
                    const int pad_spaces = col_widths[c] + padding - printed_len;

                    if (pad_spaces > 0) {
                        /* The %*s idiom right-justifies an empty string,
                           effectively printing exactly 'pad_spaces' spaces */
                        printf("%*s", pad_spaces, "");
                    }
                }
            }
        }
        printf("\n");
    }
}

/* Comparison function for alphabetizing filenames used by qsort. */
extern inline int compare_strings(const void *a, const void *b)
{
    const char *str_a = *(const char **)a;
    const char *str_b = *(const char **)b;

    return strcmp(str_a, str_b);
}

/* Comparison function for sorting by size. */
extern inline int compare_size(const void *a, const void *b) {
    struct stat buf_a;
    struct stat buf_b;
    const char *str_a = *(const char **)a;
    const char *str_b = *(const char **)b;

    if (opts.dereference) {
        if (stat(str_a, &buf_a) == -1) {
            fprintf(stderr, "%s: %s", str_a, strerror(errno));
            exit(EXIT_FAILURE);
        }
        if (stat(str_b, &buf_b) == -1) {
            fprintf(stderr, "%s: %s", str_b, strerror(errno));
            exit(EXIT_FAILURE);
        }
    } else {
        if (lstat(str_a, &buf_a) == -1) {
            fprintf(stderr, "%s: %s", str_a, strerror(errno));
            exit(EXIT_FAILURE);
        }
        if (lstat(str_b, &buf_b) == -1) {
            fprintf(stderr, "%s: %s", str_b, strerror(errno));
            exit(EXIT_FAILURE);
        }
    }

    return (buf_b.st_size > buf_a.st_size) - (buf_b.st_size < buf_a.st_size);
}

/* Comparison function for sorting by atime. */
extern inline int compare_atime(const void *a, const void *b) {
    struct stat buf_a;
    struct stat buf_b;
    const char *str_a = *(const char **)a;
    const char *str_b = *(const char **)b;

    if (opts.dereference) {
        if (stat(str_a, &buf_a) == -1) {
            fprintf(stderr, "%s: %s", str_a, strerror(errno));
            exit(EXIT_FAILURE);
        }
        if (stat(str_b, &buf_b) == -1) {
            fprintf(stderr, "%s: %s", str_b, strerror(errno));
            exit(EXIT_FAILURE);
        }
    } else {
        if (lstat(str_a, &buf_a) == -1) {
            fprintf(stderr, "%s: %s", str_a, strerror(errno));
            exit(EXIT_FAILURE);
        }
        if (lstat(str_b, &buf_b) == -1) {
            fprintf(stderr, "%s: %s", str_b, strerror(errno));
            exit(EXIT_FAILURE);
        }
    }

    return (buf_b.st_atim.tv_sec > buf_a.st_atim.tv_sec) -
           (buf_b.st_atim.tv_sec < buf_a.st_atim.tv_sec);
}

/* Comparison function for sorting by mtime. */
extern inline int compare_mtime(const void *a, const void *b) {
    struct stat buf_a;
    struct stat buf_b;
    const char *str_a = *(const char **)a;
    const char *str_b = *(const char **)b;

    if (opts.dereference) {
        if (stat(str_a, &buf_a) == -1) {
            fprintf(stderr, "%s: %s", str_a, strerror(errno));
            exit(EXIT_FAILURE);
        }
        if (stat(str_b, &buf_b) == -1) {
            fprintf(stderr, "%s: %s", str_b, strerror(errno));
            exit(EXIT_FAILURE);
        }
    } else {
        if (lstat(str_a, &buf_a) == -1) {
            fprintf(stderr, "%s: %s", str_a, strerror(errno));
            exit(EXIT_FAILURE);
        }
        if (lstat(str_b, &buf_b) == -1) {
            fprintf(stderr, "%s: %s", str_b, strerror(errno));
            exit(EXIT_FAILURE);
        }
    }

    return (buf_b.st_mtim.tv_sec > buf_a.st_mtim.tv_sec) -
           (buf_b.st_mtim.tv_sec < buf_a.st_mtim.tv_sec);
}

/* Comparison function for sorting by ctime. */
extern inline int compare_ctime(const void *a, const void *b) {
    struct stat buf_a;
    struct stat buf_b;
    const char *str_a = *(const char **)a;
    const char *str_b = *(const char **)b;

    if (opts.dereference) {
        if (stat(str_a, &buf_a) == -1) {
            fprintf(stderr, "%s: %s", str_a, strerror(errno));
            exit(EXIT_FAILURE);
        }
        if (stat(str_b, &buf_b) == -1) {
            fprintf(stderr, "%s: %s", str_b, strerror(errno));
            exit(EXIT_FAILURE);
        }
    } else {
        if (lstat(str_a, &buf_a) == -1) {
            fprintf(stderr, "%s: %s", str_a, strerror(errno));
            exit(EXIT_FAILURE);
        }
        if (lstat(str_b, &buf_b) == -1) {
            fprintf(stderr, "%s: %s", str_b, strerror(errno));
            exit(EXIT_FAILURE);
        }
    }

    return (buf_b.st_ctim.tv_sec > buf_a.st_ctim.tv_sec) -
           (buf_b.st_ctim.tv_sec < buf_a.st_ctim.tv_sec);
}

/* Reverse the sorted array for --reverse. */
extern inline void reverse_array(char* arr[], const int size) {
    int left = 0;
    int right = size - 1;

    while (left < right) {
        /* Swap the string pointers. */
        char *temp = arr[left];
        arr[left] = arr[right];
        arr[right] = temp;

        /* Move the markers toward the center. */
        left++;
        right--;
    }
}

#endif
