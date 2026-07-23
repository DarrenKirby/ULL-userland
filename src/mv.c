/***************************************************************************
 *   mv.c - move (rename) files                                            *
 *                                                                         *
 *   Copyright (C) 2014 - 2026 by Darren Kirby                             *
 *   darren@dragonbyte.ca                                                    *
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

#include <getopt.h>

#include "common.h"


static const char *APP_NAME = "mv";
static size_t PATH_MAX;

static struct {
    bool force;
    bool interactive;
    bool verbose;
} opts = { .force = false,
           .interactive = true,
           .verbose = false };

static void show_help()
{
    printf("Usage: %s [OPTION]... SOURCE DEST\n\
   or: %s [OPTION]... SOURCE... DIRECTORY\n\n\
Rename SOURCE to DEST, or move SOURCE(s) to DIRECTORY.\n\n\
Options:\n\
    -f, --force\t\tdo not prompt before overwriting\n\
    -i, --interactive\tprompt before overwrite\n\
    -h, --help\t\tdisplay this help\n\
    -V, --version\tdisplay version information\n\n\
Report bugs to <darren@dragonbyte.ca>\n", APP_NAME, APP_NAME);
}

static int prompt(char *to)
{
    printf("%s: '%s' exists. Overwrite ('y' or 'n')? ", APP_NAME, to);
    int response;
    do {
        response = getchar();
    }
    while (response == '\n');

    if (response == 'y' || response == 'Y') {
        return 1;
    }
    return 0;
}

int main(const int argc, char *argv[])
{
    const struct option long_opts[] = {
        {.name = "force",       .has_arg = 0, .flag = NULL, .val = 'f'},
        {.name = "interactive", .has_arg = 0, .flag = NULL, .val = 'i'},
        {.name = "verbose",     .has_arg = 0, .flag = NULL, .val = 'v'},
        {.name = "help",        .has_arg = 0, .flag = NULL, .val = 'h'},
        {.name = "version",     .has_arg = 0, .flag = NULL, .val = 'V'},
        {.name = NULL,          .has_arg = 0, .flag = NULL, .val = 0}
    };

    int opt;
    while ((opt = getopt_long(argc, argv, "fivVh", long_opts, NULL)) != -1) {
        switch(opt) {
            case 'v':
                opts.verbose = true;
                break;
                /* --interactive and --force are mutually exclusive.
                 * The last one passed on the CLI 'wins'. */
            case 'i':
                opts.interactive = true;
                opts.force = false;
                break;
            case 'f':
                opts.force = true;
                opts.interactive = false;
                break;
            case 'V':
                printf("%s (%s) version %s\n", APP_NAME, APP_SUITE, APP_VERSION);
                printf("%s compiled on %s at %s\n",
                       strrchr(__FILE__, '/') ? strrchr(__FILE__, '/') + 1 : __FILE__,
                       __DATE__, __TIME__);
                return EXIT_SUCCESS;
            case 'h':
                show_help();
                return EXIT_SUCCESS;
            default:
                show_help();
                return EXIT_FAILURE;
        }
    }

    PATH_MAX = get_path_max();
    int n_args = argc - optind;    /* Number of arguments */
    bool is_dir = false;           /* Final arg a directory? */
    char tmp[PATH_MAX];            /* Tmp name for 'to' when it is a directory */

    struct stat s;

    if (n_args < 2) {
        fprintf(stderr, "%s: at least two arguments required\n", APP_NAME);
        show_help();
        return EXIT_FAILURE;
    }

    char *from = argv[optind];  /* From name */
    char *to = argv[argc - 1];  /* To name */

    stat(to, &s);
    if (S_ISDIR(s.st_mode))
        is_dir = true;
    if (n_args > 2 && !is_dir) {
        printf("%s: '%s' must be a directory\n", APP_NAME, to);
        return EXIT_FAILURE;
    }

    n_args--; /* Already 'popped' last arg */

    do {
        if (is_dir) {
            snprintf(tmp, PATH_MAX, "%s/%s", to, from); /* Rename 'to' */
            if (opts.interactive && access(to, F_OK) == 0) {
                if (!prompt(to)) {
                    return EXIT_FAILURE;
                }
            }
            if (rename(from, tmp) != 0) {
                fprintf(stderr, "%s: cannot rename\n", APP_NAME);
            }
        } else {
            if (opts.interactive && access(to, F_OK) == 0) {
                if (!prompt(to)) {
                    return EXIT_FAILURE;
                }
            }
            if (rename(from, to) != 0) {
                fprintf(stderr, "%s: cannot rename\n", APP_NAME);
            }
        }
        if (opts.verbose) {
            printf("'%s' moved to '%s'\n", from, is_dir == 0 ? to : tmp);
        }
        optind++;
        n_args--;
        from = argv[optind];
    } while (n_args > 0);

    return EXIT_SUCCESS;
}
