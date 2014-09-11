/***************************************************************************
 *   yes.c - output a string repeatedly until killed                       *
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

#define APPNAME "yes"
#include "common.h"

#include <string.h>


void show_help(void) {
    printf("Usage: %s [String]...\n\
   or: %s [OPTION]\n\n \
    Repeatedly output a line with all specified STRING(s), or 'y'.\n \
    -h, --help\t\tdisplay this help\n \
    -V, --version\tdisplay version information\n\n \
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
            case 'V': printf("%s (%s) version %s\n", APPNAME, APPSUITE, APPVERSION); exit(EXIT_SUCCESS); break;
            case 'h': show_help(); exit(EXIT_SUCCESS); break;
            default : show_help(); exit(EXIT_FAILURE); break;
        }
    }
    if (argc == 1) {
        while (1 == 1)
            printf("%s\n", "y");
    } else {
        char buffer[1024];
        char *to = buffer;
        for (; optind < argc; optind++) {
            to = stpncpy(to, argv[optind], 1024);
            to = stpcpy(to, " ");
        }
        while (1 == 1)
            printf("%s\n", buffer);
    }
    return EXIT_SUCCESS;
}
