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
    #include <sys/ucred.h>
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
    -d, --include-dummy\tdisplay info for dummy file systems\n\
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
        { .name = "help",          .has_arg = no_argument, .flag = nullptr, .val = 'h' },
        { .name = "version",       .has_arg = no_argument, .flag = nullptr, .val = 'V' },
        { .name = "inodes",        .has_arg = no_argument, .flag = nullptr, .val = 'i' },
        { .name = "kilobytes",     .has_arg = no_argument, .flag = nullptr, .val = 'k' },
        { .name = "megabytes",     .has_arg = no_argument, .flag = nullptr, .val = 'm' },
        { .name = "gigabytes",     .has_arg = no_argument, .flag = nullptr, .val = 'g' },
        { .name = "sync",          .has_arg = no_argument, .flag = nullptr, .val = 's' },
        { .name = "fs-type",       .has_arg = no_argument, .flag = nullptr, .val = 'T' },
        { .name = "total",         .has_arg = no_argument, .flag = nullptr, .val = 't' },
        { .name = "include-dummy", .has_arg = no_argument, .flag = nullptr, .val = 'd' },
        { .name = nullptr,         .has_arg = no_argument, .flag = nullptr, .val = 0 }
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
        /* display all mounted file systems */
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

    n_mounts = getfsstat_ext(mfs, sizeof(struct statfs_ext) * n_mounts);
    if (n_mounts == EXIT_FAILURE) {
        fprintf(stderr, "%s: getfsstat() failed: %s\n", APP_NAME, strerror(errno));
        return EXIT_FAILURE;
    }
#endif
    /* Print the header. */
    printf("%-16s ", "Filesystem");
    if (opts.fs_type) {
        printf("%-7s ", "FS type");
    }
    if (!opts.format) {
        printf("%-*s ", 12, "1K-blocks");
    } else {
        printf("%-*s ", !opts.format || opts.format == 1 ? 12 : opts.format == 2 ? 8 : 6, "Size");
    }
    printf("%-*s ", !opts.format || opts.format == 1  ? 12 : opts.format == 2 ? 8 : 6, "Used");
    printf("%-*s ", !opts.format || opts.format == 1  ? 12 : opts.format == 2 ? 8 : 6, "Free");
    printf("%s ", "Use%");

    if (opts.inodes) {
        printf("%-12s ", "Inodes");
        printf("%-8s", "IUsed");
        printf("%s ", "IUse%");
    }

    printf("  Mount point\n");

    /* Print the data. */
    for (int i = 0; i < n_mounts; i++) {
        if (!opts.inc_dummy && mfs[i].f_blocks == 0) {
            continue;
        }

        printf("%-16s ", mfs[i].f_mntfromname);

        if (opts.fs_type) {
            printf("%-7s ", mfs[i].f_fstypename);
        }

        const uint64_t blk_1k = mfs[i].f_blocks * 4;
        const uint64_t blk_1k_free = mfs[i].f_bfree * 4;
        const uint64_t blk_1k_used = (mfs[i].f_blocks * 4 - blk_1k_free);

        const uint64_t size_bytes = mfs[i].f_blocks * mfs[i].f_bsize;
        const uint64_t free_bytes = mfs[i].f_bfree  * mfs[i].f_bsize;
        const uint64_t used_bytes = size_bytes - free_bytes;

        switch (opts.format) {
            case 1:
                printf("%-12" PRId64 " ", size_bytes / 1024);
                printf("%-12" PRId64 "  ", used_bytes / 1024);
                printf("%-12" PRId64 "  ", free_bytes / 1024);
                break;
            case 2:
                printf("%-8" PRId64 "  ", size_bytes / 1024 / 1024);
                printf("%-8" PRId64 "  ", used_bytes / 1024 / 1024);
                printf("%-8" PRId64 "  ", free_bytes / 1024 / 1024);
                break;
            case 3:
                printf("%-6" PRId64 "  ", size_bytes / 1024 / 1024 / 1024);
                printf("%-6" PRId64 "  ", used_bytes / 1024 / 1024 / 1024);
                printf("%-6" PRId64 "  ", free_bytes / 1024 / 1024 / 1024);
                break;
            default:
                printf("%-12" PRId64 "  ", blk_1k);
                printf("%-12" PRId64 "  ", blk_1k_used);
                printf("%-12" PRId64 "  ", blk_1k_free);
        }

        printf("%3" PRId64 " %% ", mfs[i].f_blocks == 0 ? 0 : calculate_percent(mfs[i].f_blocks, mfs[i].f_bfree));

        if (opts.inodes) {
            printf("%-12" PRId64 "  ", mfs[i].f_files);
            printf("%-8" PRId64 " ", mfs[i].f_files - mfs[i].f_ffree);
            printf("%3" PRId64 " %% ", mfs[i].f_blocks == 0 ? 0 : calculate_percent(mfs[i].f_files, mfs[i].f_ffree));
        }

        printf("  %s\n", mfs[i].f_mntonname);
    }

    return EXIT_SUCCESS;
}
