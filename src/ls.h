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
#ifdef __linux__
#include <sys/sysmacros.h>
#endif

#include "common.h"

#define PATH_MAX 4096

/* Decimal constant for default
 * bitfields to print. */
#define DEF_FIELDS 1008

/* Constants > 255 for long opts
 * with no associated short opt. */
#define OPT_ATIME 256
#define OPT_MTIME 257
#define OPT_CTIME 258
#define OPT_P_ATIME 259
#define OPT_P_MTIME 260
#define OPT_P_CTIME 262

/* Constants for sorting. */
#define SORT_DEFAULT 0
#define SORT_ATIME   1
#define SORT_MTIME   2
#define SORT_CTIME   3

extern char* APP_NAME;

extern inline void show_help()
{
    printf("Usage: %s [OPTION]... [FILE]...\n\n\
List and show info for files and directories\n\n\
Options:\n\
    -1, --one\t\tlist files one per line\n\
    -a, --all\t\tinclude dotfiles and implied `.' and `..' entries\n\
    -A, --almost-all\tdo not list implied `.' and `..'\n\
        --atime\t\tsort by access time, newest first\n\
    -b, --blocks\tprint allocated blocks (implies --long)\n\
    -B, --block-size\tprint I/O block size (implies --long)\n\
    -c, --colour\tprint files/dirs in colour\n\
        --ctime\t\tsort by change time, newest first\n\
    -d, --dereference\tshow information for the file links reference rather than for the link itself\n\
    -D, --device\tprint device maj/min (implies --long)\n\
    -F, --classify\tappend indicator (one of */=@|) to entries\n\
    -G, --no-group\tin a long listing, don't print group names\n\
    -h, --help\t\tdisplay this help\n\
    -H, --human\t\tdisplay file size in kilobytes and megabytes if appropriate (implies --long)\n\
    -i, --inode\t\tdisplay inode numbers (implies --long)\n\
    -l, --long\t\toutput long format listing\n\
        --mtime\t\tsort by modification time, newest first\n\
    -n, --no-colour\tdisable colour output\n\
        --print-atime\tprint access time (implies --long)\n\
        --print-ctime\tprint change time (implies --long)\n\
        --print-mtime\tprint modification time (implies --long)\n\
    -r, --reverse\treverse order while sorting\n\
    -s, --size\t\tsort by file size, largest first\n\
    -U, --no-user\tin a long listing, don't print user names\n\
    -V, --version\tdisplay version information\n\
    -w, --width=N\tset display screen width to 'N' characters\n\n\
Report bugs to <darren@dragonbyte.ca>\n", APP_NAME);
}

struct Opts {
    uint16_t fields;
    uint16_t screen_width;
    uint16_t time;
    bool ls_long;        /* Print in long-format. */
    bool human;          /* Print human-readable file sizes. */
    bool all;            /* Include dot files. */
    bool almost_all;     /* Include dot files, except for implied '.' and '..' entries. */
    bool one;            /* Print one file per line. */
    bool dereference;    /* Dereference symlinks. */
    bool colour;         /* Print files/dirs in colour. */
    bool classify;       /* Append filenames with classifiers. */
    bool size;           /* Sort by size in bytes. */
    bool reverse;        /* Print files in ascending order. */
};

/* Fields to print for -l.
 * Default: 0000001111110000 */
#define INODE_BIT 0
#define BLOCK_BIT 1
#define BLK_S_BIT 2
#define DEV_BIT   3
#define PERMS_BIT 4
#define LINKS_BIT 5
#define USER_BIT  6
#define GROUP_BIT 7
#define SIZE_BIT  8
#define MTIME_BIT 9
#define ATIME_BIT 10
#define CTIME_BIT 11

extern struct Opts opts;

extern inline int process_args(const int argc, char** argv)
{
        const struct option long_opts[] = {
         { .name = "help",        .has_arg = no_argument,       .flag = nullptr, .val = 'h' },
         { .name = "version",     .has_arg = no_argument,       .flag = nullptr, .val = 'V' },
         { .name = "all",         .has_arg = no_argument,       .flag = nullptr, .val = 'a' },
         { .name = "almost-all",  .has_arg = no_argument,       .flag = nullptr, .val = 'A' },
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
         { .name = "blocks",      .has_arg = no_argument,       .flag = nullptr, .val = 'b' },
         { .name = "block-size",  .has_arg = no_argument,       .flag = nullptr, .val = 'B' },
         { .name = "device",      .has_arg = no_argument,       .flag = nullptr, .val = 'D' },
         { .name = "no-user",     .has_arg = no_argument,       .flag = nullptr, .val = 'U' },
         { .name = "no-group",    .has_arg = no_argument,       .flag = nullptr, .val = 'G' },
         { .name = "atime",       .has_arg = no_argument,       .flag = nullptr, .val = OPT_ATIME },
         { .name = "mtime",       .has_arg = no_argument,       .flag = nullptr, .val = OPT_MTIME },
         { .name = "ctime",       .has_arg = no_argument,       .flag = nullptr, .val = OPT_CTIME },
         { .name = "print-atime", .has_arg = no_argument,       .flag = nullptr, .val = OPT_P_ATIME },
         { .name = "print-mtime", .has_arg = no_argument,       .flag = nullptr, .val = OPT_P_MTIME },
         { .name = "print-ctime", .has_arg = no_argument,       .flag = nullptr, .val = OPT_P_CTIME },
         { .name = nullptr,       .has_arg = no_argument,       .flag = nullptr, .val = 0 }
    };

    int opt;
    while ((opt = getopt_long(argc, argv, "VhalH1idw:FncsrbBDA", long_opts, nullptr)) != -1) {
        switch(opt) {
            case 'V':
                printf("%s (%s) version %s\n", APP_NAME, APP_SUITE, APP_VERSION);
                printf("%s compiled on %s at %s\n",
                       strrchr(__FILE__, '/') ? strrchr(__FILE__, '/') + 1 : __FILE__,
                       __DATE__, __TIME__);
                exit(EXIT_SUCCESS);
            case 'h':
                show_help();
                exit(EXIT_SUCCESS);
            case 'a':
                opts.all = true;
                break;
            case 'A':
                opts.almost_all = true;
                opts.all = false;
                break;
            case 'l':
                opts.ls_long = true;
                break;
            case '1':
                opts.one = true;
                break;
            case 'i':
                opts.fields |= 1 << INODE_BIT;
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
                opts.time = SORT_DEFAULT;
                break;
            case 'r':
                opts.reverse = true;
                break;
            case 'b':
                /* Print allocated blocks. */
                opts.fields |= 1 << BLOCK_BIT;
                opts.ls_long = true;   /* '-b' implies '-l' */
                break;
            case 'B':
                /* Print I/O block size. */
                opts.fields |= 1 << BLK_S_BIT;
                opts.ls_long = true;   /* '-B' implies '-l' */
                break;
            case 'D':
                /* Print device maj/min. */
                opts.fields |= 1 << DEV_BIT;
                opts.ls_long = true;   /* '-D' implies '-l' */
                break;
            case 'U':
                opts.fields &= ~(1 << USER_BIT);
                break;
            case 'G':
                opts.fields &= ~(1 << GROUP_BIT);
                break;
            case OPT_ATIME:
                opts.time = SORT_ATIME;
                break;
            case OPT_MTIME:
                opts.time = SORT_MTIME;
                break;
            case OPT_CTIME:
                opts.time = SORT_CTIME;
                break;
            case OPT_P_ATIME:
                opts.fields |= 1 << ATIME_BIT;
                opts.ls_long = true;   /* Implies '-l' */
                break;
            case OPT_P_MTIME:
                opts.fields |= 1 << MTIME_BIT;
                opts.ls_long = true;   /* Implies '-l' */
                break;
            case OPT_P_CTIME:
                opts.fields |= 1 << CTIME_BIT;
                opts.ls_long = true;   /* Implies '-l' */
                break;
            case ':':
            case '?':
            default:
                show_help();
                exit(EXIT_FAILURE);
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

extern inline char* fmt_time(const struct timespec time_d, char string_time[]) {
    time_t now_t;
    (void) time(&now_t);
    const struct tm *now = localtime(&now_t);
    const int current_year = now->tm_year + 1900;

    const struct tm *fil = localtime(&time_d.tv_sec);
    if (current_year != fil->tm_year + 1900) {
        strftime(string_time, sizeof("Jan 01  1970"), "%b %d  %Y",
                 localtime(&time_d.tv_sec));
    } else {
        strftime(string_time, sizeof("Jan 01 12:00"), "%b %d %H:%M",
                 localtime(&time_d.tv_sec));
    }
    return string_time;
}

extern inline void print_long_format(const int32_t n_files, char *filenames[])
{
    /*  We are displaying long format, one file per line. */
    struct stat buf;


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

        if (opts.fields & (1 << INODE_BIT)) {
            printf("%8" PRId64 " ", buf.st_ino);
        }

        if (opts.fields & (1 << BLOCK_BIT)) {
            printf("%3" PRId64 " ", buf.st_blocks);
        }

        if (opts.fields & (1 << BLK_S_BIT)) {
            printf("%4" PRIu32 " ", buf.st_blksize);
        }

        if (opts.fields & (1 << DEV_BIT)) {
            printf("%d/%d ", major(buf.st_dev), minor(buf.st_dev));
        }

        if (opts.fields & (1 << PERMS_BIT)) {
            printf("%s", filetype(buf.st_mode, 0));
            printf("%s ", file_perm_str(buf.st_mode, 1));
        }

        if (opts.fields & (1 << LINKS_BIT)) {
            printf("%3" PRIu32 " ", buf.st_nlink);
        }

        if (opts.fields & (1 << USER_BIT)) {
            printf("%s ", get_username(buf.st_uid));
        }

        if (opts.fields & (1 << GROUP_BIT)) {
            printf("%s ", get_groupname(buf.st_gid));
        }

        if (opts.fields & (1 << SIZE_BIT)) {
            !opts.human ? (void)printf("%6" PRId64 " ", buf.st_size) :
                format_ls(buf.st_size);    /* ie: 16k */
        }

        char t_buf[13];
        if (opts.fields & (1 << MTIME_BIT)) {
            printf(" %s ", fmt_time(buf.st_mtim, t_buf));
        }

        if (opts.fields & (1 << ATIME_BIT)) {
            printf(" %s ", fmt_time(buf.st_atim, t_buf));
        }

        if (opts.fields & (1 << CTIME_BIT)) {
            printf(" %s ", fmt_time(buf.st_ctim, t_buf));
        }

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
