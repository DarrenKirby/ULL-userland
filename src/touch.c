/***************************************************************************
 *   touch.c - update timestamp or create empty file                       *
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

/* TODO: --date, --no-dereference. Check if FreeBSD has utimensat and use that... */
/* FIXME: -a does not appear to work*/

#include "common.h"
#include <fcntl.h>
#include <sys/types.h>

const char *APPNAME = "touch";

struct opt_struct {
    unsigned int access:1;
    unsigned int modification:1;
    unsigned int nocreate:1;
    unsigned int no_dereference:1;
    unsigned int date:1;
    unsigned int reference:1;
    unsigned int current:1;
} opts;

struct timespec times[2];

static void show_help(void) {
    printf("Usage: %s [OPTION]...\n\n\
Update timestamp or create empty file\n\n\
Options:\n\
    -a, --access\t\t only change access time\n\
    -m, --modification\t\t only change modification time\n\
    -c, --nocreate\t\t don't create the file if it doesn't exist\n\
    -n, --no-dereference\t affect each symbolic link instead of any referenced file\n\
    -r, --reference=FILE\t use FILE as reference for timestamps\n\
    -h, --help\t\t\t display this help\n\
    -V, --version\t\t display version information\n\n\
Report bugs to <bulliver@gmail.com>\n", APPNAME);
}

static void to_time(char * r_file) {
    if (opts.reference == 1) {
        /* get timestamp from reference file */
        struct stat buf;

        if (stat(r_file, &buf) == -1) {
            fprintf(stderr, "stat failed on '%s'\n", r_file);
            exit(EXIT_FAILURE);
        }

        times[0].tv_sec  = buf.st_atim.tv_sec;
        times[0].tv_nsec = buf.st_atim.tv_nsec;
        times[1].tv_sec  = buf.st_mtim.tv_sec;
        times[1].tv_nsec = buf.st_mtim.tv_nsec;
    } else {
        /* for now - just use the current time */
        /* Eventually, we'll parse any arg to --date here and
         * pack it in the times struct */
        ;
    }
}

int main(const int argc, char *argv[]) {
    int opt;
    /* use the current time by default */
    opts.current = 1;
    char ref_file[PATHMAX];

    const struct option long_opts[] = {
        {"help", 0, NULL, 'h'},
        {"version", 0, NULL, 'V'},
        {"access", 0, NULL, 'a'},
        {"modification", 0, NULL, 'm'},
        {"nocreate", 0, NULL, 'c'},
        {"no-dereference", 0, NULL, 'n'},
        {"date", required_argument, NULL, 'd'},
        {"time", required_argument, NULL, 't'},
        {"reference", required_argument, NULL, 'r'},
        {NULL,0,NULL,0}
    };

    while ((opt = getopt_long(argc, argv, "Vhacmr:t:d:", long_opts, NULL)) != -1) {
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
                opts.no_dereference = 1;
                break;
            case 'r':
                opts.reference = 1;
                opts.current = 0;
                strncpy(ref_file, optarg, PATHMAX-1);
                break;
            default:
                show_help();
                exit(EXIT_FAILURE);
        }
    }

    /* populate the time struct */
    to_time(ref_file);

    while (optind < argc) {
        const int f_access = access(argv[optind], F_OK);
        if (f_access == 0) {   /* file exists */
            if (opts.current == 1) {
                if (utimensat(AT_FDCWD, argv[optind], NULL, 0) != 0) {
                    fprintf(stderr, "utimensat failed on '%s': %s\n", argv[optind], strerror(errno));
                }

            } else {
                if (utimensat(AT_FDCWD, argv[optind], times, 0) != 0) {
                    fprintf(stderr, "utimes failed on '%s': %s\n", argv[optind], strerror(errno));
                }
            }
        } else { /* file does not exist */
            if (opts.nocreate == 1) {
                /* breaking out of the iteration early misses the 'main' optind++ */
                optind++;
                continue;
            }
            /* Create the file, then close it */
            const int fd = open(argv[optind], O_CREAT, 0666);
            if (fd == -1) {
                fprintf(stderr, "open failed on '%s': %s\n", argv[optind], strerror(errno));
            }
            close(fd);
            if (opts.current == 1) {
                if (utimensat(AT_FDCWD, argv[optind], NULL, 0) != 0) {
                    fprintf(stderr, "utimensat failed: %s\n", strerror(errno));
                }
            } else {
                if (utimensat(AT_FDCWD, argv[optind], times, 0) != 0) {
                    fprintf(stderr, "utimensat failed: %s\n", strerror(errno));
                }
            }
        }
        optind++;
    }
    return EXIT_SUCCESS;
}
