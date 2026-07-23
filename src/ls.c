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
    -H, --human\t\tdisplay filesize in kilobytes and megabytes if appropriate (implies --long)\n\
    -a, --all\t\tinclude dotfiles and implied `.' and `..' entries\n\
    -1, --one\t\tlist files one per line\n\
    -i, --inode\t\tdisplay inode numbers (implies --long)\n\
    -d, --dereference\tshow information for the file links reference rather than for the link itself\n\
    -w, --width=N\tset display screen width to 'N' characters\n\
    -h, --help\t\tdisplay this help\n\
    -V, --version\tdisplay version information\n\n\
Report bugs to <darren@dragonbyte.ca>\n", APP_NAME);
}

/*
 * Return a color for a filetype
 */
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

/* Comparison function for strings */
static int compare_strings(const void *a, const void *b)
{
    return strcmp(a, b);
}

int main(const int argc, char *argv[])
{
    uint16_t screen_width = 0;
    const struct option long_opts[] = {
        {.name = "help",        .has_arg = no_argument, .flag = nullptr, .val = 'h'},
        {.name = "version",     .has_arg = no_argument, .flag = nullptr, .val = 'V'},
        {.name = "all",         .has_arg = no_argument, .flag = nullptr, .val = 'a'},
        {.name = "human",       .has_arg = 0, .flag = nullptr, .val = 'H'},
        {.name = "long",        .has_arg = 0, .flag = nullptr, .val = 'l'},
        {.name = "one",         .has_arg = 0, .flag = nullptr, .val = '1'},
        {.name = "inode",       .has_arg = 0, .flag = nullptr, .val = 'i'},
        {.name = "dereference", .has_arg = 0, .flag = nullptr, .val = 'd'},
        {.name = "width",       .has_arg = required_argument, .flag = nullptr, .val = 'w'},
        {.name = nullptr,       .has_arg = 0, .flag = nullptr, .val = 0}
    };

    int opt;
    while ((opt = getopt_long(argc, argv, "VhalH1idw:", long_opts, nullptr)) != -1) {
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
                opts.ls_long = 1;   /* '-i' implies '-l' */
                break;
            case 'd':
                opts.dereference = 1;
                break;
            case 'w':
                screen_width = (uint16_t)strtoul(optarg, nullptr, 10);
                break;
            case 'H':
                opts.human = 1;
                opts.ls_long = 1;   /* '-H' implies '-l' ls_long */
                opts.one = 1;       /* '-H' implies '-1' one     */
                break;
            default:
                show_help();
                exit(EXIT_FAILURE);
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
                exit(EXIT_FAILURE);
            }
            screen_width = w.ws_col;
        }
    }

    size_t PATH_MAX = get_path_max();
    char path_to_ls[PATH_MAX];

    if (argv[optind] != NULL) {
        strlcpy(path_to_ls, argv[optind], sizeof(path_to_ls));
    } else {
        strlcpy(path_to_ls, ".", sizeof(path_to_ls));
    }

    DIR *dp;
    struct dirent *list;
    if ((dp = opendir(path_to_ls)) == NULL) {
        fprintf(stderr, "%s: opendir failed: %s", APP_NAME, strerror(errno));
        exit(EXIT_FAILURE);
    }

    int32_t n_files = 0;           /* number of files to print */
    uint32_t n_per_line = 0;       /* number of files per line */
    uint32_t longest_so_far = 0;   /* longest filename seen so far */
    uint32_t n;                    /* return value of strlen() calls */

    while ((list = readdir(dp)) != NULL) {
        /*
         * first time around
         * get max file length
         */
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

    n_per_line = screen_width / (longest_so_far + 2); /* number of filenames per column */
    rewinddir(dp);

    char filenames[n_files][PATH_MAX];
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

    /* sort the filenames alphabetically */
    qsort(filenames, n_files, sizeof(filenames[0]), compare_strings);

    /* cd to path_to_ls */
    char cwd[PATH_MAX];
    char *cwd_p;
    cwd_p = cwd;

    if (getcwd(cwd_p, PATH_MAX) == NULL) {
        fprintf(stderr, "%s: getcwd() failed: %s\n", APP_NAME, strerror(errno));
        exit(EXIT_FAILURE);
    }

    if (chdir(path_to_ls) == -1) {
        fprintf(stderr, "%s: chdir failed: %s", APP_NAME, strerror(errno));
        exit(EXIT_FAILURE);
    }

    int f;
    if (opts.one && (!opts.ls_long)) {
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
                    fprintf(stderr, "%s: %s", filenames[f], strerror(errno));
                    exit(EXIT_FAILURE);
                }
            } else {
                if (lstat(filenames[f], &buf) == -1) {
                    fprintf(stderr, "%s: %s", filenames[f], strerror(errno));
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
#ifdef __linux__
                (void)printf("%6ld ", buf.st_size) :     /* bytes */
#else
                (void)printf("%6lld ", buf.st_size) :     /* bytes */
#endif
                format(buf.st_size) ;                     /* ie: 16k */

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
                fprintf(stderr, "%s: %s", filenames[f], strerror(errno));
                exit(EXIT_FAILURE);
            }

            printf("%s%-*s%s", file_color(buf.st_mode), (int)longest_so_far+1, filenames[f], ANSI_RESET);
            if (i % n_per_line == 0) {
                printf("\n");
            }
            i++;
        }

        if ((i-1) % n_per_line != 0) {
            printf("\n");
        }
    }

    /* Not sure if this is even necessary */
    if (chdir(cwd) == -1) {
        fprintf(stderr, "%s: %s", cwd, cwd_p);
        exit(EXIT_FAILURE); /* no biggie, already printed the output... */
    }

    return EXIT_SUCCESS;
}
