/***************************************************************************
 *   stat.c - display file attributes                                      *
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


#include <time.h>
#include <sys/types.h>

#ifdef __linux__
#include <sys/sysmacros.h>
#endif // __linux__

#include "common.h"
const char *APPNAME = "stat";

static void show_help(void) {
    printf("Usage: %s [OPTION]...\n\n\
Options:\n\
    -d, --dereference\tstat file link points to rather than link itself\n\
    -h, --help\t\tdisplay this help\n\
    -V, --version\tdisplay version information\n\n\
Report bugs to <bulliver@gmail.com>\n", APPNAME);
}

#define TIME_SIZE sizeof("1970-01-01 00:00:00.000000000 UTC")

static char *format_time(struct timespec *ts) {
    struct tm bdt;
    static char str[TIME_SIZE + 10];

    if (localtime_r(&(ts->tv_sec), &bdt) == NULL) {
        gen_error("error converting time");
        exit(EXIT_FAILURE);
    }

    snprintf(str, TIME_SIZE + 4, "%i-%02i-%02i %02i:%02i:%02i.%09li %s",
        (bdt.tm_year + 1900), bdt.tm_mon, bdt.tm_mday,
        bdt.tm_hour, bdt.tm_min, bdt.tm_sec,
        ts->tv_nsec, bdt.tm_zone
    );

    return str;
}

static void stat_file(char *filename, int follow_links) {
    struct stat buf;

    if (!follow_links) {
        if (lstat(filename, &buf) == -1) {
            f_error(filename, "couldn't stat");
            exit(EXIT_FAILURE);
        }
    } else {
        if (stat(filename, &buf) == -1) {
            f_error(filename, "couldn't stat");
            exit(EXIT_FAILURE);
        }
    }

    printf("  File: '%s'\n", filename);
    printf("  Size: %lld\t\t", (long long) buf.st_size);
    printf("Blocks: %lld\t\t", (long long)buf.st_blocks);
    printf("IO Block: %ld\t\t", (long) buf.st_blksize);
    printf("%s\n", filetype(buf.st_mode, 1));

    printf("Device: %d/%d\t\t", major(buf.st_dev), minor(buf.st_dev));
    printf("Inode: %ld\t", (long) buf.st_ino);
    printf("Links: %ld\n", (long) buf.st_nlink);

    printf(" Perms: %#o/%s\t", file_perm_oct(buf.st_mode), file_perm_str(buf.st_mode, 1));
    printf("Uid: %ld/%s\t", (long) buf.st_uid, get_username(buf.st_uid));
    printf("Gid: %ld/%s\n", (long) buf.st_gid, get_groupname(buf.st_gid));

    printf("Access: %s\n", format_time(&buf.st_atim));
    printf("Modify: %s\n", format_time(&buf.st_mtim));
    printf("Change: %s\n", format_time(&buf.st_ctim));
#ifndef linux                                          /* Linux stat struct doesn't include this */
    printf(" Birth: %s\n", format_time(&buf.st_birthtim));
#endif
}

int main(int argc, char *argv[]) {
    int opt;
    u_int follow_links = 0;

    struct option longopts[] = {
        {"help", 0, NULL, 'h'},
        {"version", 0, NULL, 'V'},
        {"dereference", 0, NULL, 'd'},
        {0,0,0,0}
    };

    while ((opt = getopt_long(argc, argv, "Vh", longopts, NULL)) != -1) {
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
            case 'd':
                follow_links = 1;
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
    while (optind < argc) {
        stat_file(argv[optind], follow_links);
        optind++;
    }
    return EXIT_SUCCESS;
}
