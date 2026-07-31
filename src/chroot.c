/***************************************************************************
 *   chroot - run command or interactive shell with special root directory *
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

#include <getopt.h>

#include "common.h"

static const char *APP_NAME = "chroot";

// struct optstruct {
// } opts;

static void show_help() {
    printf("Usage: %s [OPTION]... [NEWROOT] [COMMAND]\n\n\
run command or interactive shell with special root directory\n\n\
Options:\n\
    -h, --help\t\tdisplay this help\n\
    -V, --version\tdisplay version information\n\n\
Report bugs to <darren@dragonbyte.ca>\n", APP_NAME);
}

int main(const int argc, char *argv[])
{
    int opt;

    const struct option long_opts[] = {
        { .name = "help",    .has_arg = no_argument, .flag = nullptr, .val = 'h'},
        { .name = "version", .has_arg = no_argument, .flag = nullptr, .val = 'V'},
        { .name = nullptr,   .has_arg = no_argument, .flag = nullptr, .val = 0}
    };

    while ((opt = getopt_long(argc, argv, "Vh", long_opts, nullptr)) != -1) {
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
        default:
            show_help();
            return EXIT_FAILURE;
            break;
        }
    }
    return EXIT_SUCCESS;
}
