/***************************************************************************
 *   link - call the link function to create a link to a file              *
 *                                                                         *
 *   Copyright (C) 2014 - 2026 by Darren Kirby                             *
 *   darren@dragonbyte.ca                                                 *
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

static const char *APP_NAME = "link";

static void show_help()
{
    printf("Usage: %s OPTION\n\
   or: %s FILE1 FILE2\n\n\
Options:\n\
    -h, --help\t\tdisplay this help\n\
    -V, --version\tdisplay version information\n\n\
Report bugs to <darren@dragonbyte.ca>\n", APP_NAME, APP_NAME);
}

int main(const int argc, char *argv[]) {

    const struct option longopts[] = {
        {.name = "help",    .has_arg = no_argument, .flag = nullptr, .val = 'h'},
        {.name = "version", .has_arg = no_argument, .flag = nullptr, .val = 'V'},
        {.name = nullptr,   .has_arg = no_argument, .flag = nullptr, .val = 0}
    };

    int opt;
    while ((opt = getopt_long(argc, argv, "Vh", longopts, nullptr)) != -1) {
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
            default:
                show_help();
                return EXIT_FAILURE;
        }
    }

    if (argc <= 2 || argc >= 4) {
        show_help();
        exit(EXIT_FAILURE);
    }

    if (link(argv[optind], argv[optind+1]) != 0) {
        fprintf(stderr, "%s: link() failed: %s\n", APP_NAME, strerror(errno));
        exit(EXIT_FAILURE);
    }

    return EXIT_SUCCESS;
}
