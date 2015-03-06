/***************************************************************************
 *       *
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

#define APPNAME ""
#include "common.h"
#define _XOPEN_SOURCE 600

void show_help(void) {
    printf("Usage: %s [OPTION]...\n\n\
Options:\n\
    -h, --help\t\tdisplay this help\n\
    -V, --version\tdisplay version information\n\n\
Report bugs to <bulliver@gmail.com>\n", APPNAME);
}

int print_name(const char *path, const struct stat *stat_buf, int type, struct FTW *ftw_buf) {
    printf("%s\n", path);

    return 0;
}

int main(int argc, char *argv[]) {
    int opt;
    int recursive = 0;

    struct option longopts[] = {
        {"help", 0, NULL, 'h'},
        {"version", 0, NULL, 'V'},
        {"recursive", 0, NULL, 'R'},
        {0,0,0,0}
    };

    while ((opt = getopt_long(argc, argv, "VhR", longopts, NULL)) != -1) {
        switch(opt) {
            case 'V':
                printf("%s (%s) version %s\n", APPNAME, APPSUITE, APPVERSION);
                printf("%s compiled on %s at %s\n", basename(__FILE__), __DATE__, __TIME__);
                exit(EXIT_SUCCESS);
            case 'h':
                show_help();
                exit(EXIT_SUCCESS);
            case 'R':
                recursive = 1;
                break;
            case ':':
                 /* getopt_long prints own error message */
                exit(EXIT_FAILURE);
            case '?':
                 /* getopt_long prints own error message */
                exit(EXIT_FAILURE);
            default:
                show_help();
                exit(EXIT_FAILURE);
                break;
        }
    }

    struct group *grp_buf;

    grp_buf = getgrnam(argv[optind]);
    if (grp_buf == NULL) {
        printf("Could not resolve group name: %s", argv[optind]);
    }
    optind++;

    if (recursive == 1) {
        struct stat stat_buf;
        struct FTW	ftw_buf;

        if (nftw(argv[optind], print_name, 10, FTW_F) != 0) {
            perror("chgrp");
        }
    return EXIT_SUCCESS;
    }

    while (optind < argc) {
        //cat_file(argv[optind], line_number);
        optind++;
    }

    return EXIT_SUCCESS;
}
