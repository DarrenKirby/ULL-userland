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

#include "mount.h"
#include "common.h"

/* Print all fields.
 * 2047 = 0000011111111111 =
 * source,fstype,itotal,iused,iavail,ipcent,size,used,avail,pcent,target */
#define PRINT_ALL 2047
/* Default fields to print.
 * 1985 = 0000011111000001 =
 * source,size,used,avail,pcent,target */
#define PRINT_DEF 1985


static const char *APP_NAME = "df";

enum : uint16_t {
    source = 1 << 0,
    fstype = 1 << 1,
    itotal = 1 << 2,
    iused  = 1 << 3,
    iavail = 1 << 4,
    ipcent = 1 << 5,
    size   = 1 << 6,
    used   = 1 << 7,
    avail  = 1 << 8,
    pcent  = 1 << 9,
    target = 1 << 10
};

static struct  {
    uint16_t fields;
    bool total;
    bool inc_dummy;
    bool sync;
    uint8_t format;
} opts = {
    .fields = PRINT_DEF,
    .total = false,
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
    -a, --all\t\tdisplay info for dummy file systems\n\n\
    -[k|m|g] --[kilobytes|megabytes|gigabytes]\n\
        equivalent to -bK, -bM, -bG\n\n\
    -o, --output[=field[,]...]\n\
        Comma-delimited list of fields to print. With no arg all fields\n\
        are printed. Possible fields are: 'source', 'fstype', 'itotal',\n\
        'iused', 'iavail', 'ipcent', 'size', 'used', 'avail', 'pcent', and 'target'.\n\n\
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

typedef struct  {
    const char* name;
    uint16_t bit_pos;
} FieldMap;

static const FieldMap FIELD_MAPPINGS[] = {
    {.name = "source", .bit_pos = 0}, {.name = "fstype", .bit_pos = 1},
    {.name = "itotal", .bit_pos = 2}, {.name = "iused",  .bit_pos = 3},
    {.name = "iavail", .bit_pos = 4}, {.name = "ipcent", .bit_pos = 5},
    {.name = "size",   .bit_pos = 6}, {.name = "used",   .bit_pos = 7},
    {.name = "avail",  .bit_pos = 8}, {.name = "pcent",  .bit_pos = 9},
    {.name = "target", .bit_pos = 10}
};

static uint16_t parse_fields(char* arg)
{
    constexpr char delim[] = ",";
    uint16_t r = 0;

    char* token = strtok(arg, delim);
    while (token != NULL) {
        bool found = false;

        for (size_t i = 0; i < sizeof(FIELD_MAPPINGS) / sizeof(FIELD_MAPPINGS[0]); ++i) {
            if (strcmp(token, FIELD_MAPPINGS[i].name) == 0) {
                r |= 1 << FIELD_MAPPINGS[i].bit_pos;
                found = true;
                break;
            }
        }

        if (!found) {
            fprintf(stderr, "%s: invalid field name: %s\n", APP_NAME, token);
            exit(EXIT_FAILURE);
        }

        token = strtok(NULL, delim);
    }
    return r;
}

int main(const int argc, char *argv[])
{
    const struct option longopts[] = {
        { .name = "help",       .has_arg = no_argument,       .flag = nullptr, .val = 'h' },
        { .name = "version",    .has_arg = no_argument,       .flag = nullptr, .val = 'V' },
        { .name = "inodes",     .has_arg = no_argument,       .flag = nullptr, .val = 'i' },
        { .name = "kilobytes",  .has_arg = no_argument,       .flag = nullptr, .val = 'k' },
        { .name = "megabytes",  .has_arg = no_argument,       .flag = nullptr, .val = 'm' },
        { .name = "gigabytes",  .has_arg = no_argument,       .flag = nullptr, .val = 'g' },
        { .name = "sync",       .has_arg = no_argument,       .flag = nullptr, .val = 's' },
        { .name = "fs-type",    .has_arg = no_argument,       .flag = nullptr, .val = 'T' },
        { .name = "total",      .has_arg = no_argument,       .flag = nullptr, .val = 't' },
        { .name = "all",        .has_arg = no_argument,       .flag = nullptr, .val = 'a' },
        { .name = "output",     .has_arg = optional_argument, .flag = nullptr, .val = 'o' },
        { .name = "block-size", .has_arg = required_argument, .flag = nullptr, .val = 'b' },
        { .name = nullptr,      .has_arg = no_argument,       .flag = nullptr, .val = 0 }
    };

    int opt;
    while ((opt = getopt_long(argc, argv, "VhikmgsTtao::b:", longopts, nullptr)) != -1) {
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
            /* Zero-out bits 6-9. */
            opts.fields &= ~(0b1111 << 6);
            /* Set bits 2-5. */
            opts.fields |= 0b1111 << 2;
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
            /* Set bit 1. */
            opts.fields |= 1 << 1;
            break;
        case 't':
            opts.total = true;
            break;
        case 'a':
            opts.inc_dummy = true;
            break;
        case 'o':
            if (optarg) {
                opts.fields = parse_fields(optarg);
            } else {
                /* No arg: print everything. */
                opts.fields = PRINT_ALL;
            }
            break;
        case 'b':
            switch (optarg[0]) {
            case 'k':
            case 'K':
                opts.format = 1;
                break;
            case 'm':
            case 'M':
                opts.format = 2;
                break;
            case 'g':
            case 'G':
                opts.format = 3;
                break;
            default:
                fprintf(stderr, "%s: invalid block size specifier: %s", APP_NAME, optarg);
                return EXIT_FAILURE;
            }
            break;
        default:
            show_help();
            EXIT_FAILURE;
            break;
        }
    }

    int n_mounts = 0;
    const int args = argc - optind;
    vfs_stat_t *mfs;

    if (argc == optind) {
        n_mounts = vfs_getfsstat(nullptr, 0);

        mfs = malloc(sizeof(vfs_stat_t) * n_mounts);
        if (!mfs) {
            fprintf(stderr, "%s: malloc failed!\n", APP_NAME);
            return EXIT_FAILURE;
        }

        if (opts.sync) {
            sync();
        }

        n_mounts = vfs_getfsstat(mfs, sizeof(vfs_stat_t) * n_mounts);
        if (n_mounts == EXIT_FAILURE) {
            fprintf(stderr, "%s: getfsstat() failed: %s\n", APP_NAME, strerror(errno));
            return EXIT_FAILURE;
        }
    } else {
        /* Args passed... */
        mfs = malloc(sizeof(vfs_stat_t) * args);
        if (!mfs) {
            fprintf(stderr, "%s: malloc failed!\n", APP_NAME);
            return EXIT_FAILURE;
        }

        while (optind < argc) {
            vfs_stat_t tmp;

            /* Calls native statfs on Mac, and custom statfs_ext on Linux */
            if (vfs_statfs(argv[optind], &tmp) == -1) {
                fprintf(stderr, "%s: %s: %s\n", APP_NAME, argv[optind], strerror(errno));
                optind++;
                continue;
            }
            optind++;

            /* Deduplicate using the populated string fields */
            int is_dupe = 0;
            for (int i = 0; i < n_mounts; i++) {
                if (strcmp(mfs[i].f_mntfromname, tmp.f_mntfromname) == 0 &&
                    strcmp(mfs[i].f_mntonname, tmp.f_mntonname) == 0) {
                    is_dupe = 1;
                    break;
                    }
            }

            if (is_dupe) {
                continue;
            }

            memcpy(&mfs[n_mounts], &tmp, sizeof(vfs_stat_t));
            n_mounts++;
        }
    }
    /* Reset optind. */
    optind -= args;

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

    /* Print the selected headers. */
    if (opts.fields & source) {
        printf("%-16s ", "Filesystem");
    }

    if (opts.fields & fstype) {
        printf("%-10s ", "FS type");
    }

    if (opts.fields & itotal) {
        printf("%-12s ", "Inodes");
    }

    if (opts.fields & iused) {
        printf("%-8s ", "IUsed");
    }

    if (opts.fields & iavail) {
        printf("%-12s ", "IFree");
    }

    if (opts.fields & ipcent) {
        printf("%5s ", "IUse%");
    }

    if (opts.fields & size) {
        printf("%-*s ", w, size_label);
    }

    if (opts.fields & used) {
        printf("%-*s ", w, "Used");
    }

    if (opts.fields & avail) {
        printf("%-*s ", w, "Free");
    }

    if (opts.fields & pcent) {
        printf("%4s ", "Use%");
    }

    if (opts.fields & target) {
        printf(" %s", "Mount Point");
    }

    putchar('\n');

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

        /* Print the selected data. */
        if (opts.fields & source) {
            printf("%-16s ", mfs[i].f_mntfromname);
        }

        if (opts.fields & fstype) {
            printf("%-10s ", mfs[i].f_fstypename);
        }

        if (opts.fields & itotal) {
            printf("%-12" PRId64 " ", mfs[i].f_files);
        }

        if (opts.fields & iused) {
            printf("%-8" PRId64 " ", mfs[i].f_files - mfs[i].f_ffree);
        }

        if (opts.fields & iavail) {
            printf("%-12" PRId64 " ", mfs[i].f_ffree);
        }

        if (opts.fields & ipcent) {
            printf("%4" PRId64 "%% ", calculate_percent(mfs[i].f_files, mfs[i].f_ffree));
        }

        if (opts.fields & size) {
            printf("%-*" PRId64 " ", w, p_size);
        }

        if (opts.fields & used) {
            printf("%-*" PRId64 " ", w, p_used);
        }

        if (opts.fields & avail) {
            printf("%-*" PRId64 " ", w, p_free);
        }

        if (opts.fields & pcent) {
            printf("%3" PRId64 "%% ", calculate_percent(mfs[i].f_blocks, mfs[i].f_bfree));
        }

        if (opts.fields & target) {
            printf(" %s", mfs[i].f_mntonname);
        }

        putchar('\n');
    }

    free(mfs);
    return EXIT_SUCCESS;
}
