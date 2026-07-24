/***************************************************************************
 *   env.c - run a program in a modified environment                       *
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

/* TODO: printing and setting mostly works, but needs a lot of
 * work to run commands in the specified environment. */

#include <getopt.h>

#include "common.h"


static const char *APP_NAME = "env";

extern char **environ;

/* Cycle through and print all env variables */
static int print_all_env()
{
    while (*environ) {
        printf("%s\n", *environ);
        environ++;
    }
    return EXIT_SUCCESS;
}

static void show_help() {
    printf("Usage: %s [OPTION]... [NAME=VALUE]... [COMMAND [ARG]...]\n\n\
    Set each NAME to VALUE in the environment and run COMMAND.\n\
    With no args just print all environmental variables.\n\n\
Options:\n\
    -i, --ignore-environment   start with an empty environment\n\
    -u, --unset=NAME           remove variable from the environment\n\
    -h, --help\t\tdisplay this help\n\
    -V, --version\tdisplay version information\n\n\
Report bugs to <darren@dragonbyte.ca>\n", APP_NAME);
}

int main(const int argc, char *argv[])
{
    int es;

    const struct option longopts[] = {
        {.name = "ignore-environment", .has_arg = 0, .flag = nullptr, .val = 'i'},
        {.name = "unset",   .has_arg = 0, .flag = nullptr, .val = 'u'},
        {.name = "help",    .has_arg = 0, .flag = nullptr, .val = 'h'},
        {.name = "version", .has_arg = 0, .flag = nullptr, .val = 'V'},
        {.name = nullptr,   .has_arg = 0, .flag = nullptr, .val = 0}
    };

    int opt;
    while ((opt = getopt_long(argc, argv, "iu:Vh", longopts, NULL)) != -1) {
        switch(opt) {
            case 'i':
#ifdef __linux__
                es = clearenv();
                if (es != 0) {
                    fprintf(stderr, "clearenv failed: %s\n", strerror(errno));
                }
#else
                while (*environ) {
                    unsetenv(*environ);
                    environ++;
                }
#endif
                break;
            case 'u':
                es = unsetenv(optarg);
                if (es != 0) {
                    fprintf(stderr, "%s: could not unset %s\n", APP_NAME, optarg);
                }
                return EXIT_FAILURE;
            case 'V':
                printf("%s (%s) version %s\n", APP_NAME, APP_SUITE, APP_VERSION);
                printf("%s compiled on %s at %s\n",
                       strrchr(__FILE__, '/') ? strrchr(__FILE__, '/') + 1 : __FILE__,
                       __DATE__, __TIME__);
                return EXIT_SUCCESS;
            case 'h': show_help(); return EXIT_SUCCESS;
            default : show_help(); return EXIT_FAILURE;
        }
    }

    /* Just dump all env variables. */
    if (argc == optind) {
        return print_all_env();
    }

    return EXIT_SUCCESS;
}

