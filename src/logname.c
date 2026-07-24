/***************************************************************************
 *  logname - Print the user's login name                                  *
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

/* This is more likely capped at 32, depending on the system. */
#define LOGIN_NAME_MAX 256

static const char *APP_NAME = "logname";


static void show_help()
{
    printf("Usage: %s [OPTION]...\n\n\
Options:\n\
    -h, --help\t\t display this help\n\
    -V, --version\t display version information\n\n\
Report bugs to <darren@dragonbyte.ca>\n", APP_NAME);
}

int main(const int argc, char *argv[])
{
    const struct option long_opts[] = {
        {.name = "help",    .has_arg = 0, .flag = nullptr, .val = 'h'},
        {.name = "version", .has_arg = 0, .flag = nullptr, .val = 'V'},
        {.name = nullptr,   .has_arg = 0, .flag = nullptr, .val = 0}
    };

    int opt;
    while ((opt = getopt_long(argc, argv, "Vh", long_opts, NULL)) != -1) {
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
    char log_name[LOGIN_NAME_MAX];
    const int rv = getlogin_r(log_name, LOGIN_NAME_MAX);
    if (rv != 0) {
        fprintf(stderr, "%s: getlogin_r() failed: %s", APP_NAME, strerror(rv));
        return EXIT_FAILURE;
    }
    printf("%s\n", log_name);
    return EXIT_SUCCESS;
}
