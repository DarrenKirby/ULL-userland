/***************************************************************************
 *   vdir.c - print directory contents                                     *
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


#include <stddef.h>
#include <time.h>
#include <dirent.h>
#include <getopt.h>

#include "common.h"

static const char *APP_NAME = "vdir";

static struct {
    bool human:1;
    bool all:1;
    bool inode:1;
    bool dereference:1;
    bool colour:1;
    bool classify:1;
} opts = { .human = false,
            .all = false,
            .inode = false,
            .dereference = false,
            .colour = false,
            .classify = false };

static void show_help()
{
    printf("Usage: %s [OPTION]... [FILE]...\n\n\
Print long-form directory contents\n\n\
Options:\n\
    -H, --human\t\t display file size in kilobytes and megabytes if appropriate (implies --long)\n\
    -a, --all\t\t include dotfiles and implied `.' and `..' entries\n\
    -i, --inode\t\t display inode numbers\n\
    -d, --dereference\t show information for the file links reference rather than for the link itself\n\
    -h, --help\t\t display this help\n\
    -c, --colour\t use colour output\n\
    -F, --classify\t\tappend indicator (one of */=>@|) to entries\n\
    -V, --version\t display version information\n\n\
Report bugs to <darren@dragonbyte.ca>\n", APP_NAME);
}

static void p_colour(char * filename, const struct stat buf)
{
    switch (buf.st_mode & S_IFMT) {
        case S_IFBLK:
            printf(ANSI_YELLOW "%s" ANSI_RESET, filename); /* block device */
            break;
        case S_IFCHR:
            printf(ANSI_YELLOW_B "%s" ANSI_RESET, filename); /* character device */
            break;
        case S_IFDIR:
            printf(ANSI_BLUE_B "%s" ANSI_RESET "%s",
                filename, opts.classify ? "/" : ""); /* directory */
            break;
        case S_IFIFO:
            printf(ANSI_YELLOW "%s" ANSI_RESET "%s",
                filename, opts.classify ? "|" : ""); /* FIFO/pipe */
            break;
        case S_IFLNK:
            printf(ANSI_CYAN_B "%s" ANSI_RESET "%s",
                filename, opts.classify ? " ->" : ""); /* symlink */
            break;
        case S_IFSOCK:
            printf("%s%s", filename, opts.classify ? "=" : ""); /* socket */
            break;
        default:   /* regular file */
            /* Is it executable ? */
            if ((buf.st_mode & S_IXUSR) || (buf.st_mode & S_IXGRP) || (buf.st_mode & S_IXOTH)) {
                printf(ANSI_GREEN_B "%s" ANSI_RESET "%s", filename, opts.classify ? "*" : "");
            } else {
                printf("%s", filename);
            }
            break;
    }
}

static void p_classify(char * filename, const struct stat buf)
{
    switch (buf.st_mode & S_IFMT) {
        case S_IFBLK:
            printf("%s", filename); /* block device */
            break;
        case S_IFCHR:
            printf("%s", filename); /* character device */
            break;
        case S_IFDIR:
            printf("%s/", filename); /* directory */
            break;
        case S_IFIFO:
            printf("%s|", filename); /* FIFO/pipe */
            break;
        case S_IFLNK:
            /* FIXME: links should point to their targets. */
            printf("%s ->", filename); /* symlink */
            break;
        case S_IFSOCK:
            printf("%s=", filename); /* socket */
            break;
        default:   /* regular file */
            /* Is it executable ? */
            if ((buf.st_mode & S_IXUSR) || (buf.st_mode & S_IXGRP) || (buf.st_mode & S_IXOTH)) {
                printf("%s*", filename);
            } else {
                printf("%s", filename);
            }
            break;
    }
}

int main(const int argc, char *argv[])
{
    const struct option long_opts[] = {
        {.name = "help",        .has_arg = 0, .flag = nullptr, .val = 'h'},
        {.name = "version",     .has_arg = 0, .flag = nullptr, .val = 'V'},
        {.name = "all",         .has_arg = 0, .flag = nullptr, .val = 'a'},
        {.name = "human",       .has_arg = 0, .flag = nullptr, .val = 'H'},
        {.name = "inode",       .has_arg = 0, .flag = nullptr, .val = 'i'},
        {.name = "dereference", .has_arg = 0, .flag = nullptr, .val = 'd'},
        {.name = "colour",      .has_arg = 0, .flag = nullptr, .val = 'c'},
        {.name = "classify",    .has_arg = 0, .flag = nullptr, .val = 'F'},
        {.name = nullptr,       .has_arg = 0, .flag = nullptr, .val = 0}
    };

    int opt;
    while ((opt = getopt_long(argc, argv, "VhaHidcF", long_opts, NULL)) != -1) {
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
            case 'H':
                opts.human = true;
                break;
            case 'i':
                opts.inode = true;
                break;
            case 'd':
                opts.dereference = true;
                break;
            case 'c':
                opts.colour = true;
                break;
            case 'F':
                opts.classify = true;
                break;
            default:
                show_help();
                return EXIT_FAILURE;
        }
    }

    const size_t PATH_MAX = get_path_max();
    char path_to_ls[PATH_MAX];

    if (argv[optind] != NULL) {
        strlcpy(path_to_ls, argv[optind], sizeof(path_to_ls));
    } else {
        strlcpy(path_to_ls, ".", sizeof(path_to_ls));
    }

    DIR *dp;
    struct dirent *list;
    if ((dp = opendir(path_to_ls)) == NULL) {
        fprintf(stderr, "%s: opendir() failed: %s\n", APP_NAME, strerror(errno));
        return EXIT_FAILURE;
    }

    int n_files = 0; /* number of files to print */

    while ((list = readdir(dp)) != NULL) {
        /* First time around get max file length. */
        if (!opts.all) {
            if (list->d_name[0] == '.') {
                continue;
            }
        }
        n_files++;
    }

    rewinddir(dp);

    char filenames[n_files][PATH_MAX];
    int n = 0;

    while ((list = readdir(dp)) != NULL) {
        if (opts.all == 0) {
            if (list->d_name[0] == '.') {
                continue;
            }
        }
        snprintf(filenames[n], sizeof filenames[n], "%s", list->d_name);
        n++;
    }
    closedir(dp);

    char cwd[PATH_MAX];
    char *cwd_p = cwd;

    if (getcwd(cwd_p, PATH_MAX) == NULL) {
        fprintf(stderr, "%s: getcwd() failed: %s\n", APP_NAME, strerror(errno));
        return EXIT_FAILURE;
    }

    if (chdir(path_to_ls) == -1) {
        fprintf(stderr, "%s: chdir() failed: %s\n", APP_NAME, strerror(errno));
        return EXIT_FAILURE;
    }

    struct stat buf;
    time_t now_t;
    (void) time(&now_t);
    const struct tm *now = localtime(&now_t);
    const int current_year = now->tm_year + 1900;
    char string_time[13];

    for (int f = 0; f < n_files; f++) {
        if (opts.dereference == 1) {
            if (stat(filenames[f], &buf) == -1) {
                fprintf(stderr, "%s: stat() failed: %s\n", APP_NAME, strerror(errno));
                return EXIT_FAILURE;
            }
        } else {
            if (lstat(filenames[f], &buf) == -1) {
                fprintf(stderr, "%s: lstat() failed: %s\n", APP_NAME, strerror(errno));
                return EXIT_FAILURE;
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
            (void)printf("%6lld ", (long long) buf.st_size) :       /* bytes */
            format(buf.st_size) ;                  /* ie: 16k */

        const struct tm *fil = localtime(&buf.st_mtime);
        if (current_year != (fil->tm_year + 1900)) {
            strftime(string_time, sizeof("Jan 01  1970"), "%b %d  %Y", localtime(&buf.st_mtime));
        } else {
            strftime(string_time, sizeof("Jan 01 12:00"), "%b %d %H:%M", localtime(&buf.st_mtime));
        }

        printf("%s ", string_time);

        if (opts.colour) {
            p_colour(filenames[f], buf);
        } else if (opts.classify) {
            p_classify(filenames[f], buf);
        } else {
            printf("%s", filenames[f]);
        }
        printf("\n");
    }

    return EXIT_SUCCESS;
}
