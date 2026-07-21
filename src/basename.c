/***************************************************************************
 *   basename.c - strip directory and suffix from paths/filenames          *
 *                                                                         *
 *   Copyright (C) 2014-2025 by Darren Kirby                               *
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

#include <getopt.h>
#include <libgen.h>

#include "common.h"


static const char *APP_NAME = "basename";

static void show_help() {
    printf("Usage: %s PATH\n\
   or: %s [OPTION] [PATH]\n\n\
Strip leading directories and optional suffix from PATH.\n\n\
Options:\n\
    -s, --suffix=SUFFIX\t remove trailing suffix\n\
    -h, --help\t\t\t display this help\n\
    -V, --version\t\t display version information\n\n\
    Examples:\n\
    \t basename /usr/bin/sort         Output: 'sort'.\n\
    \t basename -s .h include/stdio.h Output: 'stdio'. \n\n\
Report bugs to <darren@dragonbyte.ca>\n", APP_NAME, APP_NAME);
}

int main(const int argc, char *argv[]) {
    size_t PATH_MAX = get_path_max() + 1;
    char name[PATH_MAX];
    char suffix[PATH_MAX];
    bool sfx = false;
    bool sdn = false;

    const struct option long_opts[] = {
        {"suffix", required_argument, nullptr, 's'},
        {"help", 0, nullptr, 'h'},
        {"version", 0, nullptr, 'V'},
        {nullptr,0,nullptr,0}
    };

    int opt;
    while ((opt = getopt_long(argc, argv, "Vs:h", long_opts, nullptr)) != -1) {
        switch(opt) {
            case 's':
                sfx = true;
                strncpy(suffix, optarg, PATH_MAX);
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

    /* Number of arguments */
    const int n_args = argc - optind;

    /* Read path from STDIN */
    if (n_args == 0 || strcmp(argv[optind], "-") == 0) {
        if (fgets(name, (int)PATH_MAX, stdin) == NULL) {
            fprintf(stderr, "Error reading stdin\n");
            return EXIT_FAILURE;
        }
        sdn = true;
    } else {
	    strncpy(name, argv[optind], PATH_MAX);
    }

    /* Strip the trailing suffix */
    if (sfx) {
        char *np = name + strlen(name);
        const char *sp = suffix + strlen(suffix);

        while (np > name && sp > suffix) {
            if (*--np != *--sp) {
                return 0;
            }
        }
        if (np > name) {
            *np = '\0';
        }
    }
    printf("%s%s", basename(name), sdn == 0 ? "\n" : "");
    return EXIT_SUCCESS;
}
