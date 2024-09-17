/***************************************************************************
 *   ln.c - make links between files                                       *
 *                                                                         *
 *   Copyright (C) 2014 - 2024 by Darren Kirby                             *
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

#include <unistd.h>

#include "common.h"

const char *APPNAME = "ln";

static void show_help(void) {
    printf("Usage: %s [OPTION]... TARGET LINK_NAME\t(1st form)\n \
  or: %s [OPTION]... TARGET\t\t(2nd form)\n\n \
    -s, --symbolic\tmake symbolic links instead of hard links\n \
    -f, --force\tdo not prompt before overwriting files\n \
    -i, --interactive\tprompt before overwriting files\n \
    -v, --verbose\tprint out links created\n \
    -h, --help\t\tdisplay this help\n \
    -V, --version\tdisplay version information\n\n \
    Report bugs to <bulliver@gmail.com>\n", APPNAME, APPNAME);
}

struct optstruct {
    unsigned int symbolic:1;
    unsigned int force:1;
    unsigned int verbose:1;
    unsigned int interactive:1;
} opts;

int main(int argc, char *argv[]) {
    int opt;

    struct option longopts[] = {
        {"symbolic",    0, NULL, 's'},
        {"force",       0, NULL, 'f'},
        {"interactive", 0, NULL, 'i'},
        {"verbose",     0, NULL, 'v'},
        {"help",        0, NULL, 'h'},
        {"version",     0, NULL, 'V'},
        {0,0,0,0}
    };

    while ((opt = getopt_long(argc, argv, "sfivVh", longopts, NULL)) != -1) {
        switch(opt) {
            case 'V':
                printf("%s (%s) version %s\n", APPNAME, APPSUITE, APPVERSION);
                printf("%s compiled on %s at %s\n", basename(__FILE__), __DATE__, __TIME__);
                exit(EXIT_SUCCESS);
            case 'h':
                show_help();
                exit(EXIT_SUCCESS);
            case 's':
                opts.symbolic = 1;
                break;
            case 'f':
                opts.force = 1;
                break;
            case 'v':
                opts.verbose = 1;
                break;
            case 'i':
                opts.interactive = 1;
                break;
            case ':':
                 /* getopt_long prints own error message */
                exit(EXIT_FAILURE);
            case '?':
                 /* getopt_long prints own error message */
                exit(EXIT_FAILURE);
            default :
                show_help();
                exit(EXIT_FAILURE);
        }
    }
    /* FIXME: existing file will get clobbered even without -f */

    if ((argc - optind) == 2) { /* 1st form */
        if (access(argv[optind], F_OK) == 0) {
            if (opts.force)
                unlink(argv[optind + 1]);
            if (opts.interactive) {
                char response;
                printf("`%s' exists, overwrite? ('y' or 'n') ", argv[optind + 1]);
                do {
                response = getchar();
                } while (response == '\n');

                if (response == 'y' || response == 'Y')
                    unlink(argv[optind + 1]);
                else
                    exit(EXIT_FAILURE);
            }
        }

        if (opts.symbolic) {
            if (symlink(argv[optind], argv[optind + 1]) == -1) {
                perror("symlink");
                exit(EXIT_FAILURE);
            }
        } else {
            if (link(argv[optind], argv[optind + 1]) == -1) {
                perror("link");
                exit(EXIT_FAILURE);
            }
        }

        if (opts.verbose)
            printf("linked `%s' to `%s'\n", argv[optind], argv[optind + 1]);


    } else if ((argc - optind)== 1) { /* 2nd form */
        char target[PATHMAX];
        char *target_p = target;
        target_p = basename(argv[optind]);

        if (access(argv[optind], F_OK) == 0) {
            if (opts.force)
                unlink(target_p);
            if (opts.interactive) {
                char response;
                printf("`%s' exists, overwrite? ('y' or 'n') ", target_p);
                do {
                response = getchar();
                } while (response == '\n');

                if (response == 'y' || response == 'Y')
                    unlink(target_p);
                else
                    exit(EXIT_FAILURE);
            }
        }

        if (opts.symbolic) {
            if (symlink(argv[optind], target_p) == -1) {
                perror("symlink");
                exit(EXIT_FAILURE);
            }
        } else {
            if (link(argv[optind], target_p) == -1) {
                perror("link");
                exit(EXIT_FAILURE);
            }
        }

        if (opts.verbose)
            printf("linked `%s' to `%s'\n", argv[optind], target_p);

    } else {
        perror("not enough arguments");
        exit(EXIT_FAILURE);
    }

    return EXIT_SUCCESS;
}
