/***************************************************************************
 *   env.c - run a program in a modified environment                       *
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



#include "common.h"

const char *APPNAME = "env";

extern char **environ;

/* Cycle through and print all env variables */
static int print_all_env(void) {

    while (*environ) {
        printf("%s\n", *environ);
        environ++;
    }
    return EXIT_SUCCESS;
}

static void show_help(void) {
    printf("Usage: %s [OPTION]... [NAME=VALUE]... [COMMAND [ARG]...]\n\n\
    Set each NAME to VALUE in the environment and run COMMAND.\n\n\
Options:\n\
    -i, --ignore-environment   start with an empty environment\n\
    -u, --unset=NAME           remove variable from the environment\n\
    -h, --help\t\tdisplay this help\n\
    -V, --version\tdisplay version information\n\n\
Report bugs to <bulliver@gmail.com>\n", APPNAME);
}

int main(int argc, char *argv[]) {
    int opt, es;

    struct option longopts[] = {
        {"ignore-environment", 0, NULL, 'i'},
        {"unset", 0, NULL, 'u'},
        {"help", 0, NULL, 'h'},
        {"version", 0, NULL, 'V'},
        {0,0,0,0}
    };

    while ((opt = getopt_long(argc, argv, "iu:Vh", longopts, NULL)) != -1) {
        switch(opt) {
            case 'i':
#if defined (__linux__)
                es = clearenv();
                if (es != 0) {
                    gen_error("Could not clear environment");
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
                    char message[50];
                    sprintf(message, "Cound not unset %s", optarg);
                    gen_error(message);
                }
                break;
            case 'V':
                printf("%s (%s) version %s\n", APPNAME, APPSUITE, APPVERSION);
                printf("%s compiled on %s at %s\n", basename(__FILE__), __DATE__, __TIME__);
                exit(EXIT_SUCCESS);
            case 'h': show_help(); exit(EXIT_SUCCESS);
            default : show_help(); exit(EXIT_FAILURE);
        }
    }

    if (argc == optind) {      /* Just dump all env variables */
        if (print_all_env() != 0) {
            perror("print_all_env");
            exit(EXIT_FAILURE);
        }
    }

    return EXIT_SUCCESS;
}

