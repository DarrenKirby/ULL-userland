/***************************************************************************
 *   rmdir.c -  remove empty directories                                   *
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
const char *APPNAME = "rmdir";

void show_help(void) {
    printf("Usage: %s [OPTION]... DIRECTORY...\n\n\
    Remove the DIRECTORY(ies), if they are empty.\n\n\
Options:\n\
    -v, --verbose\toutput a diagnostic for every directory processed\n\
    -h, --help\t\tdisplay this help\n \
    -V, --version\tdisplay version information\n\n \
Report bugs to <bulliver@gmail.com>\n", APPNAME);
}

int main(int argc, char *argv[]) {
    int opt;
    unsigned int verbose = 0;

    struct option longopts[] = {
        {"verbose", 0, NULL, 'v'},
        {"help", 0, NULL, 'h'},
        {"version", 0, NULL, 'V'},
        {0,0,0,0}
    };

    while ((opt = getopt_long(argc, argv, "vVh", longopts, NULL)) != -1) {
        switch(opt) {
            case 'v':
                verbose = 1;
                break;
            case 'V':
                printf("%s (%s) version %s\n", APPNAME, APPSUITE, APPVERSION);
                printf("%s compiled on %s at %s\n", basename(__FILE__), __DATE__, __TIME__);
                exit(EXIT_SUCCESS);
                break;
            case 'h':
                show_help();
                exit(EXIT_SUCCESS);
                break;
            default:
                show_help();
                exit(EXIT_FAILURE);
                break;
        }
    }

    for (; optind < argc; optind++) {
        if (rmdir(argv[optind]) != 0){
            f_error(argv[optind], "Cannot remove ");
        }
        if (verbose) {
            printf("%s: Removed directory '%s'\n", APPNAME, argv[optind]);
        }
    }
    return EXIT_SUCCESS;
}
