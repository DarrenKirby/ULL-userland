/***************************************************************************
 *   stat.c - display file attributes                                      *
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

#include <getopt.h>
#include <time.h>
#ifdef __linux__
#include <sys/sysmacros.h>
#endif

#include "common.h"


static const char *APP_NAME = "stat";

static void show_help()
{
    printf("Usage: %s [OPTION]... FILE [FILE]...\n\n\
Display file attributes\n\n\
Options:\n\
    -d, --dereference\t stat the file the link points to rather than the link itself\n\
    -h, --help\t\t display this help\n\
    -V, --version\t display version information\n\n\
Report bugs to <darren@dragonbyte.ca>\n", APP_NAME);
}

#define TIME_SIZE sizeof("1970-01-01 00:00:00.000000000 UTC")

static char *format_time(const struct timespec *ts)
{
    struct tm bdt;
    static char str[TIME_SIZE + 10];

    if (localtime_r(&ts->tv_sec, &bdt) == NULL) {
        /* FIXME: localtime_r's return value is not documented.
         * Does it even set errno? */
        fprintf(stderr, "%s: localtime_r failed: %s\n", APP_NAME, strerror(errno));
        exit(EXIT_FAILURE);
    }

    snprintf(str, TIME_SIZE + 4, "%i-%02i-%02i %02i:%02i:%02i.%09li %s",
        (bdt.tm_year + 1900), bdt.tm_mon, bdt.tm_mday,
        bdt.tm_hour, bdt.tm_min, bdt.tm_sec,
        ts->tv_nsec, bdt.tm_zone
    );
    return str;
}

static void stat_file(char *filename, const int follow_links)
{
    struct stat buf;
    const size_t PATH_MAX = get_path_max();

    if (!follow_links) {
        if (lstat(filename, &buf) == -1) {
            fprintf(stderr, "%s: lstat() failed: %s\n", APP_NAME, strerror(errno));
            exit(EXIT_FAILURE);
        }
    } else {
        if (stat(filename, &buf) == -1) {
            fprintf(stderr, "%s: stat() failed: %s\n", APP_NAME, strerror(errno));
            exit(EXIT_FAILURE);
        }
    }

    if ((buf.st_mode & S_IFMT) == S_IFLNK) {
        char link_target[PATH_MAX];
        if (readlink(filename, link_target, PATH_MAX) == -1) {
            fprintf(stderr, "%s: readlink() failed: %s\n", APP_NAME, strerror(errno));
            printf("Link target unknown\n");
            printf("  File: '%s'\n", filename);
        }
        printf("  File: '%s' -> %s\n", filename, link_target);
    } else {
        printf("  File: '%s'\n", filename);
    }
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
#ifndef linux  /* Linux stat struct doesn't include this */
    printf(" Birth: %s\n", format_time(&buf.st_birthtim));
#endif
}

int main(const int argc, char *argv[])
{
    bool follow_links = false;

    const struct option long_opts[] = {
        {.name = "help",        .has_arg = 0, .flag = nullptr, .val = 'h'},
        {.name = "version",     .has_arg = 0, .flag = nullptr, .val = 'V'},
        {.name = "dereference", .has_arg = 0, .flag = nullptr, .val = 'd'},
        {.name = nullptr,       .has_arg = 0, .flag = nullptr, .val = 0}
    };

    int opt;
    while ((opt = getopt_long(argc, argv, "Vhd", long_opts, nullptr)) != -1) {
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
            case 'd':
                follow_links = true;
                break;
            default:
                show_help();
                return EXIT_FAILURE;
        }
    }

    const int args = argc - optind;
    if (!args) {
        fprintf(stderr, "%s: missing arguments\n", APP_NAME);
        show_help();
        return EXIT_FAILURE;
    }

    while (optind < argc) {
        stat_file(argv[optind], follow_links);
        optind++;
    }
    return EXIT_SUCCESS;
}
