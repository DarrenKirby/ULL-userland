/***************************************************************************
 *   mkdir.c - create a new empty directory                                *
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

#define APPNAME "mkdir"
#include "common.h"
#include <sys/stat.h>

void show_help(void) {
    printf("Usage: %s [OPTION]...\n\n \
    -h, --help\t\tdisplay this help\n \
    -V, --version\tdisplay version information\n \
    -v, --verbose\tdisplay directories created\n\n \
    Report bugs to <bulliver@gmail.com>\n", APPNAME);
}

int main(int argc, char *argv[]) {
    int opt;
    mode_t mode;
    mode = 0755;
    int verbose = 0;

    static struct option longopts[] = {
        {"verbose", 0, NULL, 'v'},
        {"mode", required_argument, NULL, 'm'},
        {"help", 0, NULL, 'h'},
        {"version", 0, NULL, 'V'},
        {0,0,0,0}
    };

    while ((opt = getopt_long(argc, argv, "vm:hV", longopts, NULL)) != -1) {
        switch(opt) {
            case 'v':
                verbose = 1;
                break;
            case 'm':
                errno = 0;
                mode = strtoul(optarg, NULL, 8); 
                if (errno != 0)
                    printf("%s: invalid mode agrument: %s", APPNAME, optarg);
                    exit(EXIT_FAILURE);
                break;
            case 'V': 
                printf("%s (%s) version %s\n", APPNAME, APPSUITE, APPVERSION); 
                exit(EXIT_SUCCESS); 
                break;
            case 'h': 
                show_help(); 
                exit(EXIT_SUCCESS); 
                break;
            case ':':
                printf("option '%c' needs an argument\n", optopt);
                exit(EXIT_FAILURE);
                break;
            case '?':
                /* getopt_long prints own error message */
                exit(EXIT_FAILURE);
            default: 
                show_help(); 
                exit(EXIT_FAILURE); 
                break;
        }
    }

    if (argc == optind)
        g_error("must supply at least one directory argument");
    
    while (optind < argc) {
        mkdir(argv[optind], mode);
        if (verbose == 1)
            printf("%s: created directory '%s'\n", APPNAME, argv[optind]);
        optind++;
    }
    return EXIT_SUCCESS;
}
