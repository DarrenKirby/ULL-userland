/***************************************************************************
 *   mv.c - move (rename) files                                            *
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


#include <unistd.h>
#include "common.h"

const char *APPNAME = "mv";

struct optstruct {
    int force;
    int interactive;
    int verbose;
} opts;

static void show_help(void) {
    printf("Usage: %s [OPTION]... SOURCE DEST\n\
    or: %s [OPTION]... SOURCE... DIRECTORY\n\n\
    Rename SOURCE to DEST, or move SOURCE(s) to DIRECTORY.\n\n\
Options:\n\
    -f, --force\t\tdo not prompt before overwriting\n\
    -i, --interactive\tprompt before overwrite\n\
    -h, --help\t\tdisplay this help\n\
    -V, --version\tdisplay version information\n\n\
Report bugs to <bulliver@gmail.com>\n", APPNAME, APPNAME);
}

static int prompt(char *to) {
    printf("%s: %s exists. Overwrite ('y' or 'n')? ", APPNAME, to);
    char response;
    do {
        response = getchar();
    }
    while (response == '\n');

    if (response == 'y' || response == 'Y') {
        return 1;
    } else {
        return 0;
    }
}

int main(int argc, char *argv[]) {
    int opt;

    struct option longopts[] = {
        {"force", 0, NULL, 'f'},
        {"interactive", 0, NULL, 'i'},
        {"verbose", 0, NULL, 'v'},
        {"help", 0, NULL, 'h'},
        {"version", 0, NULL, 'V'},
        {0,0,0,0}
    };

    while ((opt = getopt_long(argc, argv, "fivVh", longopts, NULL)) != -1) {
        switch(opt) {
            case 'v':
                opts.verbose = 1;
                break;
            case 'i':
                opts.interactive = 1;
                break;
            case 'f':
                opts.force = 1;
                break;
            case 'V':
                printf("%s (%s) version %s\n", APPNAME, APPSUITE, APPVERSION);
                printf("%s compiled on %s at %s\n",
                       strrchr(__FILE__, '/') ? strrchr(__FILE__, '/') + 1 : __FILE__,
                       __DATE__, __TIME__);
                exit(EXIT_SUCCESS);
            case 'h':
                show_help();
                exit(EXIT_SUCCESS);
            default:
                show_help();
                exit(EXIT_FAILURE);
        }
    }

    int n_args = argc - optind;    /* Number of arguments */
    int isdir = 0;                 /* Final arg a directory? */
    char *from;                    /* From name */
    char *to;                      /* To name */
    char tmp[FILEMAX + 1];         /* Tmp name for 'to' when it is a directory */

    struct stat s;

    if (n_args < 2) {
        gen_error("at least two arguments required\n");
        show_help();
        exit(EXIT_FAILURE);
    }

    from = argv[optind];
    to = argv[argc-1];

    stat(to, &s);
    if (S_ISDIR(s.st_mode))
        isdir = 1;
    if (n_args > 2 && isdir == 0) {
        printf("%s: '%s' must be a directory\n", APPNAME, to);
        exit(EXIT_FAILURE);
    }

    n_args--; /* Already 'popped' last arg */

    do {
        if (isdir == 1) {

            snprintf(tmp, 256, "%s/%s", to, from); /* Rename 'to' */
            if (opts.interactive && access(to, F_OK) == 0) {
                if (!prompt(to)) {
                    exit(EXIT_FAILURE);
                }
            }
            if (rename(from, tmp) != 0) {
                f_error(to, NULL);
            }
        } else {
            if (opts.interactive && access(to, F_OK) == 0) {
                if (!prompt(to)) {
                    exit(EXIT_FAILURE);
                }
            }
            if (rename(from, to) != 0) {
                f_error(to, NULL);
            }
        }
        if (opts.verbose) {
            printf("'%s' -> '%s'\n", from, isdir == 0 ? to : tmp);
        }
        optind++;
        n_args--;
        from = argv[optind];
    } while (n_args > 0);

    return EXIT_SUCCESS;
}

