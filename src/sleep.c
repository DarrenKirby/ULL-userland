/***************************************************************************
 *   sleep.c - delay for a specified amount of time                        *
 *                                                                         *
 *   Copyright (C) 2014 by Darren Kirby                                    *
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

#define APPNAME "sleep"
#include "common.h"

#include <unistd.h>

#define MINUTES 60
#define HOURS 3600
#define DAYS 86400

/* TODO: Allow decimal arguments and multiple arguments */

void show_help(void) {
    printf("Usage: %s [OPTION]\n\
   or: %s NUMBER[SUFFIX]\n\n\
    Pause for NUMBER seconds. SUFFIX may be 's' for seconds (the default),\n\
    'm' for minutes, 'h' for hours or 'd' for days.\n\n\
Options:\n\
    -h, --help\t\tdisplay this help\n\
    -V, --version\tdisplay version information\n\n\
Report bugs to <bulliver@gmail.com>\n", APPNAME, APPNAME);
}

long int make_seconds(char *int_string) {
    long val;
    char *endptr;

    val = strtol(int_string, &endptr, 10);
    if (*endptr != '\0') {
        if (strcmp(endptr, "s") == 0)
            return val;
        if (strcmp(endptr, "m") == 0)
            return (val * MINUTES);
        if (strcmp(endptr, "h") == 0)
            return (val * HOURS);
        if (strcmp(endptr, "d") == 0)
            return (val * DAYS);
    }
    return val;
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
                exit(EXIT_SUCCESS);
            case 'h':
                show_help();
                exit(EXIT_SUCCESS);
            default:
                show_help();
                exit(EXIT_FAILURE);
        }
    }
    long int sleep_length;
    sleep_length = make_seconds(argv[1]);
    sleep(sleep_length);
    return EXIT_SUCCESS;
}
