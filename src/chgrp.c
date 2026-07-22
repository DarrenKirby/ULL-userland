/***************************************************************************
 *   chgrp.c - change group ownership of file                              *
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

#include <sys/types.h>
#include <sys/stat.h>
#include <fts.h>
#include <getopt.h>

#include "common.h"

#define MAX_GROUP_LEN 32


static const char *APP_NAME = "chgrp";

static struct group *grp_buf;
static char *to_grp;

static struct {
    bool no_dereference;
    bool recursive;
    bool verbose;
} opts = {.no_dereference = false, .recursive = false, .verbose = false};

static void show_help() {
    printf("Usage: %s [OPTION] group FILE [FILE]...\n\n\
Change group ownership of file(s)\n\n\
Options:\n\
    -R, --recursive\t\tchange group of files recursively\n\
    -v, --verbose\t\toutput a diagnostic for every file processed\n\
    -d, --no-dereference\toperate on symbolic links rather than their targets\n\
    -h, --help\t\t\tdisplay this help\n\
    -V, --version\t\tdisplay version information\n\n\
Report bugs to <darren@dragonbyte.ca>\n", APP_NAME);
}

/* Compare function for FTS. */
static int cmp(const FTSENT **s1, const FTSENT **s2)
{
    return strcoll((*s1)->fts_name, (*s2)->fts_name);
}

static int chgrp_recurse(char *path) {
    FTSENT *f;
    char  *argv[] = { path , nullptr };

    const int flags = opts.no_dereference ? FTS_PHYSICAL : FTS_LOGICAL;
    FTS *tree = fts_open(argv, flags | FTS_NOSTAT, cmp);
    if (!tree) {
        fprintf(stderr, "fts_open() failed: %s\n", strerror(errno));
        return -1;
    }

    while ((f = fts_read(tree))) {
        switch (f->fts_info) {
        case FTS_DNR:
            fprintf(stderr, "could not read '%s': %s\n", f->fts_path, strerror(f->fts_errno));
            continue;
        case FTS_ERR:
            fprintf(stderr, "failed on '%s': %s", f->fts_path, strerror(f->fts_errno));
            /* Intentional fall-through to the continue... */
        case FTS_DP:
            continue;
        default:
            if (f->fts_info == FTS_SL && opts.no_dereference == 1) {
                if (lchown(f->fts_path, -1, grp_buf->gr_gid) != 0) {
                    fprintf(stderr, "lchown failed on '%s'\n", path);
                }
            } else {
                if (chown(f->fts_path, -1, grp_buf->gr_gid) != 0) {
                    fprintf(stderr, "chown failed on '%s'\n", path);
                }
            }
        }
        if (opts.verbose) {
            printf("Changed group ownership of '%s' to '%s'\n", f->fts_path, to_grp);
        }
    }

    fts_close(tree);
    return 0;
}

int main(const int argc, char *argv[]) {
    const struct option long_opts[] = {
        {.name = "help", .has_arg = no_argument, .flag = nullptr, .val = 'h'},
        {.name = "version", .has_arg = no_argument, .flag = nullptr, .val = 'V'},
        {.name = "recursive", .has_arg = no_argument, .flag = nullptr, .val = 'R'},
        {.name = "verbose", .has_arg = no_argument, .flag = nullptr, .val = 'v'},
        {.name = "no-dereference", .has_arg = no_argument, .flag = nullptr, .val = 'd'},
        {.name = nullptr, .has_arg = 0, .flag = nullptr, .val = 0}
    };

    int opt;
    while ((opt = getopt_long(argc, argv, "VhRvd", long_opts, nullptr)) != -1) {
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
            case 'R':
                opts.recursive = true;
                break;
            case 'v':
                opts.verbose = true;
                break;
            case 'd':
                opts.no_dereference = true;
                break;
            default:
                show_help();
                return EXIT_FAILURE;
                break;
        }
    }

    to_grp = malloc(MAX_GROUP_LEN);
    if (!to_grp) {
        fprintf(stderr, "malloc failed\n");
        return EXIT_FAILURE;
    }
    strncpy(to_grp, argv[optind], MAX_GROUP_LEN - 1);

    grp_buf = getgrnam(argv[optind]);
    if (grp_buf == NULL) {
        printf("Could not resolve group name: %s", argv[optind]);
        return EXIT_FAILURE;
    }
    optind++;

    if (opts.recursive) {
        /* Status of chgrp calls checked within chgrp_recursive()
         * A failure of any particular file will be noted, but we
         * will continue to run... */
        if (chgrp_recurse(argv[optind]) != 0) {
            return EXIT_FAILURE;
        }
        return EXIT_SUCCESS;
    }

    while (optind < argc) {
        if (opts.recursive) {
            /* Status of chgrp calls checked within chgrp_recursive()
             * A failure of any particular file will be noted, but we
             * will continue to run... */
            chgrp_recurse(argv[optind++]);
            continue;
        }

        if (opts.no_dereference) {
            if (lchown(argv[optind], -1, grp_buf->gr_gid) != 0) {
                fprintf(stderr, "lchown failed on `%s'\n", argv[optind]);
            }
        } else {
            if (chown(argv[optind], -1, grp_buf->gr_gid) != 0) {
                fprintf(stderr, "chown failed on `%s'\n", argv[optind++]);
            }
        }

        if (opts.verbose) {
            printf("Changed group ownership of `%s' to `%s'\n", argv[optind++], to_grp);
        }
    }
    return EXIT_SUCCESS;
}
