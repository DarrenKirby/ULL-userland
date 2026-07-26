/***************************************************************************
 *   touch.c - update timestamp or create empty file                       *
 *                                                                         *
 *   Copyright (C) 2014 - 2026 by Darren Kirby                             *
 *   darren@dragonbyte.ca                                                  *
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

/* TODO: implement --date */

/* NOTE: The apparent efficacy of this program will differ in regard to
 *       a_time modification between *BSD/macOS and Linux due to their
 *       respective kernel's willingness to tolerate logical inconsistencies.
 *       To wit: Linux will let you set a file's mtime more recent than
 *       the file's atime, whereas the *BSD/macOS kernels enforce logic
 *       that says if the file was modified, it must have been accessed.
 *       The upshot is that some systems may also update the atime even if
 *       you specify (using -m) to only update mtime.
 */

#include <getopt.h>
#include <fcntl.h>

#include "common.h"


static const char *APP_NAME = "touch";

static struct {
    bool access:1;
    bool modification:1;
    bool nocreate:1;
    bool no_dereference:1;
    bool date:1;
    bool reference:1;
} opts = {
    .access = false,
    .modification = false,
    .nocreate = false, 
    .reference = false,
    .date = false, 
    .no_dereference = false };

static struct timespec times[2];

static void show_help()
{
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
Report bugs to <darren@dragonbyte.ca>\n", APP_NAME);
}

static void to_time(char * r_file)
{
    if (opts.reference) {
        struct stat buf;

        if (stat(r_file, &buf) == -1) {
            fprintf(stderr, "stat failed on '%s'\n", r_file);
            exit(EXIT_FAILURE);
        }
        /* get timestamp from reference file */
        times[0].tv_sec  = buf.st_atim.tv_sec;
        times[0].tv_nsec = buf.st_atim.tv_nsec;
        times[1].tv_sec  = buf.st_mtim.tv_sec;
        times[1].tv_nsec = buf.st_mtim.tv_nsec;

        if (opts.access) {
            times[1].tv_nsec = UTIME_OMIT;
        }
        if (opts.modification) {
            times[0].tv_nsec = UTIME_OMIT;
        }

    } else {
        /* Eventually, we'll parse any arg to --date here and
         * pack it in the times struct */

        /* These fields should be ignored, but the utimensat
         * manpage references a bug in old Linux kernel versions
         * that will error if the values are not set to 0 */
        times[0].tv_sec = 0;
        times[1].tv_sec = 0;

        if (opts.access) {
            times[0].tv_nsec = UTIME_NOW;
            times[1].tv_nsec = UTIME_OMIT;
        } else if (opts.modification) {
            times[0].tv_nsec = UTIME_OMIT;
            times[1].tv_nsec = UTIME_NOW;
        } else {
            times[0].tv_nsec = UTIME_NOW;
            times[1].tv_nsec = UTIME_NOW;
        }
    }
}

int main(const int argc, char *argv[])
{
    const struct option long_opts[] = {
        { .name = "help",           .has_arg = no_argument,       .flag = nullptr, .val = 'h' },
        { .name = "version",        .has_arg = no_argument,       .flag = nullptr, .val = 'V' },
        { .name = "access",         .has_arg = no_argument,       .flag = nullptr, .val = 'a' },
        { .name = "modification",   .has_arg = no_argument,       .flag = nullptr, .val = 'm' },
        { .name = "nocreate",       .has_arg = no_argument,       .flag = nullptr, .val = 'c' },
        { .name = "no-dereference", .has_arg = no_argument,       .flag = nullptr, .val = 'n' },
        { .name = "date",           .has_arg = required_argument, .flag = nullptr, .val = 'd' },
        { .name = "reference",      .has_arg = required_argument, .flag = nullptr, .val = 'r' },
        { .name = nullptr,          .has_arg = no_argument,       .flag = nullptr, .val = 0}
    };
    
    const size_t path_max = get_path_max();
    char ref_file[path_max];

    int opt;
    while ((opt = getopt_long(argc, argv, "Vhancmr:d:", long_opts, nullptr)) != -1) {
        switch(opt) {
            case 'V':
                printf("%s (%s) version %s\n", APP_NAME, APP_SUITE, APP_VERSION);
                printf("%s compiled on %s at %s\n",
                       strrchr(__FILE__, '/') ? strrchr(__FILE__, '/') + 1 : __FILE__,
                       __DATE__, __TIME__);
                return EXIT_SUCCESS;
            case 'h':
                show_help();
                return EXIT_SUCCESS;
            case 'a':
                /* mutually exclusive */
                opts.access = true;
                opts.modification = false;
                break;
            case 'm':
                /* mutually exclusive */
                opts.modification = true;
                opts.access = false;
                break;
            case 'c':
                opts.nocreate = true;
                break;
            case 'n':
                opts.no_dereference = true;
                break;
            case 'r':
                opts.reference = true;
                snprintf(ref_file, sizeof(ref_file), "%s", optarg);
                break;
            default:
                show_help();
                return EXIT_FAILURE;
        }
    }

    /* Populate the reference time struct. */
    to_time(ref_file);
    /* Are we dereferencing symlinks? */
    const int dr_flag = opts.no_dereference ? AT_SYMLINK_NOFOLLOW : 0;

    while (optind < argc) {
        const int f_access = access(argv[optind], F_OK);
        if (f_access == 0) {   /* file exists */
            if (utimensat(AT_FDCWD, argv[optind], times, dr_flag) != 0) {
                fprintf(stderr, "%s: utimes() failed on '%s': %s\n",
                    APP_NAME, argv[optind], strerror(errno));
                continue;
            }
        } else { /* File does not exist. */
            if (opts.nocreate == 1) {
                /* Breaking out of the iteration early misses the 'main' optind++ */
                optind++;
                continue;
            }
            /* Create the file, then close it. */
            const int fd = open(argv[optind], O_CREAT, 0666);
            if (fd == -1) {
                fprintf(stderr, "%s: open() failed on '%s': %s\n",
                    APP_NAME, argv[optind], strerror(errno));
                continue;
            }
            close(fd);
            if (utimensat(AT_FDCWD, argv[optind], times, dr_flag) != 0) {
                fprintf(stderr, "%s: utimensat failed on '%s': %s\n",
                    APP_NAME, argv[optind], strerror(errno));
                continue;
            }
        }
        optind++;
    }
    return EXIT_SUCCESS;
}
