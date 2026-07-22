/***************************************************************************
 *   dirname - strip non-directory suffix from file name                   *
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

#include <getopt.h>
#include <libgen.h>

#include "common.h"


static const char *APP_NAME = "dirname";

static void show_help() {
    printf("Usage: %s OPTION\n\
   or: %s PATH [PATH...]\n\n\
Options:\n\
    -h, --help\t\tdisplay this help\n\
    -V, --version\tdisplay version information\n\n\
Report bugs to <darren@dragonbyte.ca>\n", APP_NAME, APP_NAME);
}

int main(int argc, char *argv[]) {
    int opt;

    const struct option longopts[] = {
        {.name = "help",    .has_arg = 0, .flag = nullptr, .val = 'h'},
        {.name = "version", .has_arg = 0, .flag = nullptr, .val = 'V'},
        {.name = nullptr,   .has_arg = 0, .flag = nullptr, .val = 0}
    };

    while ((opt = getopt_long(argc, argv, "Vh", longopts, nullptr)) != -1) {
        switch(opt) {
            case 'V':
                printf("%s (%s) version %s\n", APP_NAME, APP_SUITE, APP_VERSION);
                printf("%s compiled on %s at %s\n",
                       strrchr(__FILE__, '/') ? strrchr(__FILE__, '/') + 1 : __FILE__,
                       __DATE__, __TIME__);
                return EXIT_SUCCESS;
                break;
            case 'h':
                show_help();
                return EXIT_SUCCESS;
                break;
            case ':':
                 /* getopt_long prints own error message */
                return EXIT_FAILURE;
                break;
            case '?':
                 /* getopt_long prints own error message */
                return EXIT_FAILURE;
            default:
                show_help();
                return EXIT_FAILURE;
                break;
        }
    }

    if (argc == 1) {
        show_help();
        return EXIT_FAILURE;
    }

    char *d;
    while (optind < argc) {
        if ((d = dirname(argv[optind])) == NULL) {
            perror("dirname");
            return EXIT_FAILURE;
        }

        printf("%s\n", d);
        optind++;
    }

    return EXIT_SUCCESS;
}
