/***************************************************************************
 *   touch.c - update timestamp or create empty file                       *
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


#define APPNAME "touch"
#include "common.h"

struct optstruct {
    int access;
    int modification;
    int nocreate;
} opts;

static void show_help(void) {
    printf("Usage: %s [OPTION]...\n\n\
Options:\n\
    -a, --access\tonly change access time\n\
    -m, --modification\tonly change modification time\n\
    -c, --nocreate\tdon't create the file if it doesn't exist\n\
    -h, --help\t\tdisplay this help\n\
    -V, --version\tdisplay version information\n\n\
Report bugs to <bulliver@gmail.com>\n", APPNAME);
}

int main(int argc, char *argv[]) {
    int opt;

    struct option longopts[] = {
        {"help", 0, NULL, 'h'},
        {"version", 0, NULL, 'V'},
        {"access", 0, NULL, 'a'},
        {"modification", 0, NULL, 'm'},
        {"nocreate", 0, NULL, 'c'},
        {0,0,0,0}
    };

    while ((opt = getopt_long(argc, argv, "Vhacmr:t:d:", longopts, NULL)) != -1) {
        switch(opt) {
            case 'V':
                printf("%s (%s) version %s\n", APPNAME, APPSUITE, APPVERSION);
                printf("%s compiled on %s at %s\n", basename(__FILE__), __DATE__, __TIME__);
                exit(EXIT_SUCCESS);
                break;
            case 'h':
                show_help();
                exit(EXIT_SUCCESS);
                break;
            case 'a':
                opts.modification = 1;
                break;
            case 'm':
                opts.access = 1;
                break;
            case 'c':
                opts.nocreate = 1;
                break;
            case ':':
                 /* getopt_long prints own error message */
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

    int fd;
    //char *p = 0; // NULL pointer
    //struct timespec amtimes;
    //amtimes.tv_sec  = ;
    //amtimes.tv_nsec = UTIME_NOW;


    while (optind < argc) {
        fd = open(argv[optind], O_CREAT | O_EXCL | O_WRONLY, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH);
        if (fd < 0) {
            if (errno == EEXIST) {    /* file exists */
                if (utimes(argv[optind], NULL) != 0) {
                    perror("touch");
                }

            } else {
                perror("touch");
            }

        } else { /* file does not exist */
            if (opts.nocreate == 1) {
                exit(EXIT_FAILURE);
            }

            if (futimes(fd, NULL) != 0) {
                perror("touch");
            }

        }
        optind++;
    }

    return EXIT_SUCCESS;
}
