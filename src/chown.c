/***************************************************************************
 *   chown.c - change ownership of files                                   *
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

/* TODO: allow for passing uid/gid. */

#include <fts.h>
#include <getopt.h>

#include "common.h"


static const char *APP_NAME = "chown";
static size_t FILE_MAX;

static struct group *grp_buf;
static struct passwd *own_buf;

static struct {
    bool no_dereference;
    bool recursive;
    bool verbose;
    bool group_too;
} opts = {
    .no_dereference = false,
    .recursive = false,
    .verbose = false,
    .group_too = false };

static void show_help(void) {
    printf("Usage: %s [OPTION] user[:group]...\n\n\
Change ownership of files\n\n\
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

static int chown_recurse(char *path, char to_own[], char to_grp[])
{
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
                fprintf(stderr, "%s: could not read '%s': %s\n", APP_NAME, f->fts_path, strerror(f->fts_errno));
                continue;
            case FTS_ERR:
                fprintf(stderr, "%s: failed on '%s': %s", APP_NAME, f->fts_path, strerror(f->fts_errno));
                /* Intentional fall-through to the continue... */
            case FTS_DP:
                continue;
            default:
                if (opts.group_too) {
                    if (f->fts_info == FTS_SL && opts.no_dereference) {
                        if (lchown(f->fts_path, own_buf->pw_uid, grp_buf->gr_gid) != 0) {
                            fprintf(stderr, "%s: lchown failed on '%s'\n", APP_NAME, path);
                        }
                    } else {
                        if (chown(f->fts_path, own_buf->pw_uid, grp_buf->gr_gid) != 0) {
                            fprintf(stderr, "%s: chown failed on '%s'\n", APP_NAME, path);
                        }
                    }
                } else {
                    if (f->fts_info == FTS_SL && opts.no_dereference) {
                        if (lchown(f->fts_path, own_buf->pw_uid, -1) != 0) {
                            fprintf(stderr, "%s: lchown failed on '%s'\n", APP_NAME, path);
                        }
                    } else {
                        if (chown(f->fts_path, own_buf->pw_uid, -1) != 0) {
                            fprintf(stderr, "%s: chown failed on '%s'\n", APP_NAME, path);
                        }
                    }
                }
        }
        if (opts.verbose) {
            printf("Changed ownership of '%s' to '%s'\n", f->fts_path, to_own);
            if (opts.group_too) {
                printf("Changed group ownership of '%s' to '%s'\n", f->fts_path, to_grp);
            }
        }
    }

    fts_close(tree);
    return 0;
}

int main(const int argc, char *argv[])
{
    const struct option long_opts[] = {
        {.name = "help",           .has_arg = 0, .flag = nullptr, .val = 'h'},
        {.name = "version",        .has_arg = 0, .flag = nullptr, .val = 'V'},
        {.name = "recursive",      .has_arg = 0, .flag = nullptr, .val = 'R'},
        {.name = "verbose",        .has_arg = 0, .flag = nullptr, .val = 'v'},
        {.name = "no-dereference", .has_arg = 0, .flag = nullptr, .val = 'd'},
        {.name = nullptr,.has_arg = 0, .flag = nullptr,.val = 0}
    };

    int opt;
    while ((opt = getopt_long(argc, argv, "VhRvd", long_opts, NULL)) != -1) {
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
        }
    }

    FILE_MAX = get_filename_max();

    char to_grp[FILE_MAX];
    char to_own[FILE_MAX];

    if (strchr(argv[optind], 58) != NULL) {
        /* user:group */
        const char *po = strtok(argv[optind], ":");
        const char *pg = strtok(NULL, ":");
        strncpy(to_own, po, FILE_MAX);
        strncpy(to_grp, pg, FILE_MAX);

        opts.group_too = true;
    } else {
        /* only specified user (new owner) */
        strncpy(to_own, argv[optind], FILE_MAX);
    }
    optind++;

    own_buf = getpwnam(to_own);
    if (own_buf == NULL) {
        fprintf(stderr, "%s: could not resolve user name: %s\n", APP_NAME, to_own);
        return EXIT_FAILURE;
    }

    grp_buf = getgrnam(to_grp);
    if (opts.group_too && grp_buf == NULL) {
        fprintf(stderr, "%s: could not resolve group name: %s\n", APP_NAME, to_grp);
        return EXIT_FAILURE;
    }

    if (opts.recursive) {
        /* Status of chown calls checked within chown_recursive()
        * A failure of any particular file will be noted, but we
        * will continue to run... */
        if (chown_recurse(argv[optind], to_own, to_grp) != 0) {
            return EXIT_FAILURE;
        }
        return EXIT_SUCCESS;
    }

    while (optind < argc) {
        if (opts.no_dereference) {
            if (opts.group_too) {
                if (lchown(argv[optind], own_buf->pw_uid, grp_buf->gr_gid) != 0) {
                fprintf(stderr, "%s: lchown failed: %s\n", APP_NAME, strerror(errno));
                }
            } else {
                if (lchown(argv[optind], own_buf->pw_uid, -1) != 0) {
                fprintf(stderr, "%s: lchown failed: %s\n", APP_NAME, strerror(errno));
                }
            }

        } else {
            if (opts.group_too) {
                if (chown(argv[optind], own_buf->pw_uid, grp_buf->gr_gid) != 0) {
                fprintf(stderr, "%s: chown failed: %s\n", APP_NAME, strerror(errno));
                }
            } else {
                if (chown(argv[optind], own_buf->pw_uid, -1) != 0) {
                fprintf(stderr, "%s: chown failed: %s\n", APP_NAME, strerror(errno));
                }
            }
        }

        if (opts.verbose) {
            printf("changed file ownership of `%s' to `%s'\n", argv[optind], to_own);
            if (opts.group_too == 1) {
                printf("changed group ownership of `%s' to `%s'\n", argv[optind], to_grp);
            }
        }
        optind++;
    }
    return EXIT_SUCCESS;
}
