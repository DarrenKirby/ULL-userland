/***************************************************************************
 *   mkdir.c - create a new empty directory                                *
 *                                                                         *
 *   Copyright (C) 2014 - 2026 by Darren Kirby                             *
 *   darren@dagonbyte.ca                                                   *
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

static const char *APP_NAME = "mkdir";

static void show_help()
{
    printf("Usage: %s [OPTION]...\n\n \
    -h, --help\t\tdisplay this help\n \
    -V, --version\tdisplay version information\n \
    -m, --mode=MODE\tset file mode (as in chmod), not a=rwx - umask\n \
    -v, --verbose\tdisplay directories created\n\n \
    Report bugs to <darren@dragonbyte.ca>\n", APP_NAME);
}

int main(const int argc, char *argv[]) {
    int opt;
    umask(0); /* so our permissions are set as expected */
    mode_t mode = 0755; /* sensible default for dirs */
    int verbose = 0;

    static struct option longopts[] = {
        {.name = "verbose", .has_arg = no_argument,       .flag = nullptr, .val = 'v'},
        {.name = "mode",    .has_arg = required_argument, .flag = nullptr, .val = 'm'},
        {.name = "help",    .has_arg = no_argument,       .flag = nullptr, .val = 'h'},
        {.name = "version", .has_arg = no_argument,       .flag = nullptr, .val = 'V'},
        {.name = nullptr,   .has_arg = no_argument,       .flag = nullptr, .val = 0}
    };

    while ((opt = getopt_long(argc, argv, "vm:hV", longopts, nullptr)) != -1) {
        switch(opt) {
            case 'v':
                verbose = 1;
                break;
            case 'm':
                mode = strtoul(optarg, nullptr, 8);
                break;
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
            default:
                show_help();
                return EXIT_FAILURE;
                break;
        }
    }

    if (argc == optind)
        fprintf(stderr, "must supply at least one directory argument\n");

    while (optind < argc) {
        mkdir(argv[optind], mode);
        if (verbose == 1)
            printf("%s: created directory '%s'\n", APP_NAME, argv[optind]);
        optind++;
    }
    return EXIT_SUCCESS;
}


