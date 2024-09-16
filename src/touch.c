/***************************************************************************
 *   touch.c - update timestamp or create empty file                       *
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

#include <fcntl.h>
#include <sys/time.h>

#include "common.h"
const char *APPNAME = "touch";

struct optstruct {
    unsigned int access:1;
    unsigned int modification:1;
    unsigned int nocreate:1;
    unsigned int nodereference:1;
    unsigned int time:1;
    unsigned int date:1;
    unsigned int reference:1;
    unsigned int current:1;
} opts;

#if defined (__linux__)
struct timespec times[2];
#else
struct timeval times[2];
#endif

static void show_help(void) {
    printf("Usage: %s [OPTION]...\n\n \
Options:\n \
    -a, --access\tonly change access time\n \
    -m, --modification\tonly change modification time\n \
    -c, --nocreate\tdon't create the file if it doesn't exist\n \
    -n, --nodereference\taffect each symbolic link instead of any referenced file\n \
    -h, --help\t\tdisplay this help\n\
    -V, --version\tdisplay version information\n\n\
Report bugs to <bulliver@gmail.com>\n", APPNAME);
}

static void to_time(char * r_file) {
    if (opts.reference == 1) {
        /* get timestamp from reference file */
        struct stat buf;

        if (stat(r_file, &buf) == -1) {
            f_error(r_file, "couldn't stat");
            exit(EXIT_FAILURE);
        }

#if defined (__linux__)
        times[0].tv_sec  = buf.st_atim.tv_sec;
        times[0].tv_nsec = buf.st_atim.tv_nsec;
        times[1].tv_sec  = buf.st_mtim.tv_sec;
        times[1].tv_nsec = buf.st_mtim.tv_nsec;
#else
        times[0].tv_sec  = buf.st_atim.tv_sec;
        times[0].tv_usec = buf.st_atim.tv_nsec;
        times[1].tv_sec  = buf.st_mtim.tv_sec;
        times[1].tv_usec = buf.st_mtim.tv_nsec;
#endif
    } else if (opts.date == 1) {
        // parse time
    } else if (opts.time == 1) {
        //parse date
    }
}

int main(int argc, char *argv[]) {
    int opt;
    opts.current = 1;
    char r_file[PATHMAX + 1];

    struct option longopts[] = {
        {"help", 0, NULL, 'h'},
        {"version", 0, NULL, 'V'},
        {"access", 0, NULL, 'a'},
        {"modification", 0, NULL, 'm'},
        {"nocreate", 0, NULL, 'c'},
        {"nodereference", 0, NULL, 'n'},
        {"date", required_argument, NULL, 'd'},
        {"time", required_argument, NULL, 't'},
        {"reference", required_argument, NULL, 'r'},
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
            case 'a':
                opts.modification = 0;
                break;
            case 'm':
                opts.access = 0;
                break;
            case 'c':
                opts.nocreate = 1;
                break;
            case 'n':
                opts.nodereference = 1;
                break;
            case 'r':
                opts.reference = 1;
                opts.current = 0;
                strncpy(r_file, optarg, PATHMAX);
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
        }
    }

    /* populate the time struct */
    to_time(r_file);

    int fd;
    while (optind < argc) {
        fd = open(argv[optind], O_CREAT | O_EXCL | O_WRONLY, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH);
        if (fd < 0) {
            if (errno == EEXIST) {    /* file exists */
                if (opts.current == 1) {
                    if (utimes(argv[optind], NULL) != 0)
                        perror("utimes");
                } else {
                    if (utimes(argv[optind], times) != 0)
                        perror("utimes");
                }
            } else {
                perror("touch");
            }

        } else { /* file does not exist */
            if (opts.nocreate == 1) {
                exit(EXIT_FAILURE);
            }

            if (opts.current == 1) {
                if (futimes(fd, NULL) != 0)
                    perror("futimes");
            } else {
                if (futimes(fd, times) != 0)
                    perror("futimes");
            }
        }
        optind++;
    }

    return EXIT_SUCCESS;
}
