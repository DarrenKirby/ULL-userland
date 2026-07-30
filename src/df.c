/***************************************************************************
 *   df.c - report file system disk space usage                            *
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
#include <stdint.h>
#include <inttypes.h>
#if defined(__APPLE__) && defined(__MACH__)
    #define _DARWIN_FEATURE_64_BIT_INODE 1
    #define APPLE_FS 26
#endif
#if defined(__APPLE__) && defined(__MACH__) || defined(__FreeBSD__)
    #include <sys/param.h>
    #include <sys/mount.h>
#else
    #include "mount.h"
#endif

#include "common.h"


static const char *APP_NAME = "df";

static struct  {
    bool total;
    bool inodes;
    bool fs_type;
    bool inc_dummy;
    bool sync;
    uint8_t format;
} opts = {
    .total = false,
    .inodes = false,
    .fs_type = false,
    .inc_dummy = false,
    .sync = false,
    .format = 0 };

static void show_help()
{
    printf("Usage: %s [OPTION]...\n\n\
Options:\n\
    -h, --help\t\tdisplay this help\n\
    -V, --version\tdisplay version information\n\
    -i, --inodes\tdisplay information for inodes\n\
    -s, --sync\t\tsync all I/O before retrieving FS info\n\
    -T, --fs-type\tdisplay file type name\n\
    -a, --all\t\tdisplay info for dummy file systems\n\
    -[k|m|g] --[kilobytes|megabytes|gigabytes]\n\
    \t\t\tdisplay in unit rather than blocks\n\
Report bugs to <darren@dragonbyte.ca>\n", APP_NAME);
}

static uint64_t calculate_percent(const uint64_t total, const uint64_t free)
{
    if (total == 0) {
        return 0;
    }

    const uint64_t used = total - free;
    const uint64_t result = (used * 100 + (total - 1)) / total;
    return result;
}

int main(const int argc, char *argv[])
{
    const struct option longopts[] = {
        { .name = "help",      .has_arg = no_argument, .flag = nullptr, .val = 'h' },
        { .name = "version",   .has_arg = no_argument, .flag = nullptr, .val = 'V' },
        { .name = "inodes",    .has_arg = no_argument, .flag = nullptr, .val = 'i' },
        { .name = "kilobytes", .has_arg = no_argument, .flag = nullptr, .val = 'k' },
        { .name = "megabytes", .has_arg = no_argument, .flag = nullptr, .val = 'm' },
        { .name = "gigabytes", .has_arg = no_argument, .flag = nullptr, .val = 'g' },
        { .name = "sync",      .has_arg = no_argument, .flag = nullptr, .val = 's' },
        { .name = "fs-type",   .has_arg = no_argument, .flag = nullptr, .val = 'T' },
        { .name = "total",     .has_arg = no_argument, .flag = nullptr, .val = 't' },
        { .name = "all",       .has_arg = no_argument, .flag = nullptr, .val = 'a' },
        { .name = nullptr,     .has_arg = no_argument, .flag = nullptr, .val = 0 }
    };

    int opt;
    while ((opt = getopt_long(argc, argv, "VhikmgsTtd", longopts, nullptr)) != -1) {
        switch(opt) {
            case 'V':
                printf("%s (%s) version %s\n", APP_NAME, APP_SUITE, APP_VERSION);
                printf("%s compiled on %s at %s\n",
                       strrchr(__FILE__, '/') ? strrchr(__FILE__, '/') + 1 : __FILE__,
                       __DATE__, __TIME__);
                return EXIT_SUCCESS;
                break;
            case 'h':
                show_help();
                return EXIT_SUCCESS;
                break;
            case 'i':
                opts.inodes = true;
                break;
            case 'k':
                opts.format = 1;
                break;
            case 'm':
                opts.format = 2;
                break;
            case 'g':
                opts.format = 3;
                break;
            case 's':
                opts.sync = true;
                break;
            case 'T':
                opts.fs_type = true;
                break;
            case 't':
                opts.total = true;
                break;
            case 'd':
                opts.inc_dummy = true;
                break;
            default:
                show_help();
                EXIT_FAILURE;
                break;
        }
    }

    int n_mounts = 0;
#if defined(__APPLE__) && defined(__MACH__)
    if (argc == optind) {
        /* Display all mounted file systems */
        n_mounts = getfsstat(nullptr, 0, MNT_NOWAIT);
    }
    struct statfs *mfs = malloc(sizeof(struct statfs) * n_mounts);
    if (!mfs) {
        fprintf(stderr, "%s: malloc failed!\n", APP_NAME);
        return EXIT_FAILURE;
    }

    if (opts.sync) {
        sync();
    }

    if (getfsstat(mfs, sizeof(struct statfs) * n_mounts, MNT_DWAIT) == -1) {
        fprintf(stderr, "%s: getfsstat() failed: %s\n", APP_NAME, strerror(errno));
        return EXIT_FAILURE;
    }
#else
    if (argc == optind) {
        n_mounts = getfsstat_ext(nullptr, 0);
    }

    printf("n_mounts = %d\n", n_mounts);

    struct statfs_ext *mfs = malloc(sizeof(struct statfs_ext) * n_mounts);
    if (!mfs) {
        fprintf(stderr, "%s: malloc failed!\n", APP_NAME);
        return EXIT_FAILURE;
    }

    if (opts.sync) {
        sync();
    }

    n_mounts = getfsstat_ext(mfs, sizeof(struct statfs_ext) * n_mounts);
    if (n_mounts == EXIT_FAILURE) {
        fprintf(stderr, "%s: getfsstat() failed: %s\n", APP_NAME, strerror(errno));
        return EXIT_FAILURE;
    }
#endif
    /* Determine column widths and math divisors upfront */
    int w = 12;
    uint64_t div = 1;
    const char *size_label = "1K-blocks";

    if (opts.format > 0) {
        if (opts.format == 1) {
            size_label = "Size KB";
            w = 12;
            div = 1024;
        } else if (opts.format == 2) {
            size_label = "Size MB";
            w = 8;
            div = 1024 * 1024;
        } else if (opts.format == 3) {
            size_label = "SizeGB";
            w = 6;
            div = 1024 * 1024 * 1024;
        }
    }

    /* Print the Header using the dynamic width 'w' */
    printf("%-16s ", "Filesystem");
    if (opts.fs_type) {
        printf("%-7s ", "FS type");
    }

    /* Use %-*s to pass the width variable 'w', followed by ONE space. */
    printf("%-*s %-*s %-*s %4s   Mount point\n",
           w, size_label,
           w, "Used",
           w, "Free",
           "Use%");

    /* Print the Data */
    for (int i = 0; i < n_mounts; i++) {
        if (!opts.inc_dummy && mfs[i].f_blocks == 0) {
            continue;
        }

        uint64_t p_size, p_used, p_free;

        /* Calculate raw bytes first. */
        const uint64_t size_bytes = mfs[i].f_blocks * mfs[i].f_bsize;
        const uint64_t free_bytes = mfs[i].f_bfree  * mfs[i].f_bsize;
        const uint64_t used_bytes = size_bytes - free_bytes;

        /* Calculate display values based on format. */
        if (opts.format == 0) {
            p_size = size_bytes / 1024;
            p_free = free_bytes / 1024;
            p_used = used_bytes / 1024;
        } else {
            p_size = size_bytes / div;
            p_free = free_bytes / div;
            p_used = used_bytes / div;
        }

        const uint64_t pct = mfs[i].f_blocks == 0 ? 0 : calculate_percent(mfs[i].f_blocks, mfs[i].f_bfree);

        /* Print row headers. */
        printf("%-16s ", mfs[i].f_mntfromname);
        if (opts.fs_type) {
            printf("%-7s ", mfs[i].f_fstypename);
        }

        /* Print sizes using the exact same 'w' dynamic width and ONE space. */
        printf("%-*" PRId64 " %-*" PRId64 " %-*" PRId64 " %3" PRId64 "%%   %s\n",
               w, p_size,
               w, p_used,
               w, p_free,
               pct,
               mfs[i].f_mntonname);
    }

    return EXIT_SUCCESS;
}
