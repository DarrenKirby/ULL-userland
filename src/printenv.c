/***************************************************************************
 *   printenv.c - print all or part of environment                         *
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

#include "common.h"

const char *APPNAME = "printenv";

extern char **environ;

/* Cycle through and print all env variables */
static int print_all_env(void) {

    while (*environ) {
        printf("%s\n", *environ);
        environ++;
    }
    return EXIT_SUCCESS;
}

/* Print a single env variable */
static int print_env(char *name) {
    char *var, *value;
    var = name;
    value = getenv(var);
    if (value)
        printf("%s\n", value);
    else
        return EXIT_FAILURE;
    return EXIT_SUCCESS;
}

static void show_help(void) {
    printf("Usage: %s [VARIABLE]...\n\
   or: %s OPTION\n\n\
    Print the value of VARIABLE.\n\
    If no environment VARIABLE specified, print them all.\n\n\
Options:\n\
    -h, --help\t\tdisplay this help\n\
    -V, --version\tdisplay version information\n\n\
Report bugs to <bulliver@gmail.com>\n", APPNAME, APPNAME);
}

int main(int argc, char *argv[]) {
    int opt;

    struct option longopts[] = {
        {"help", 0, NULL, 'h'},
        {"version", 0, NULL, 'V'},
        {0,0,0,0}
    };

    while ((opt = getopt_long(argc, argv, "Vh", longopts, NULL)) != -1) {
        switch(opt) {
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

    if (argc == 2) {
        print_env(argv[1]);
    }else if (argc == 1) {
        print_all_env();
    } else {
        fprintf(stderr, "%s takes either zero or one argument\n", APPNAME);
        show_help(); exit(EXIT_FAILURE);
    }
    return EXIT_SUCCESS;
}

