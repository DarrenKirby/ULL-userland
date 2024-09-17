/***************************************************************************
 *   chown.c - change ownerhip of files                                    *
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




/* Needed for nftw() */
#ifdef __linux__
#define _XOPEN_SOURCE 500
#endif

#include <ftw.h>
#include <unistd.h>

#include "common.h"

const char *APPNAME = "chown";

struct group *grp_buf;
struct passwd *own_buf;
char to_grp[FILEMAX + 1];
char to_own[FILEMAX + 1];

char *pg;
char *po;

struct optstruct {
    unsigned int nodereference;
    unsigned int recursive;
    unsigned int verbose;
    unsigned int group_too;
} opts;


static void show_help(void) {
    printf("Usage: %s [OPTION] user[:group]...\n\n\
Options:\n\
    -R, --recursive\t\tchange group of files recursively\n\
    -v, --verbose\t\toutput a diagnostic for every file processed\n\
    -d, --no-dereference\t\t\n\
    -h, --help\t\tdisplay this help\n\
    -V, --version\tdisplay version information\n\n\
Report bugs to <bulliver@gmail.com>\n", APPNAME);
}

static int chown_recurse(const char *path, const struct stat *stat_buf, int type, struct FTW *ftw_buf) {
    if (type == FTW_NS) {
        printf("stat failed on `%s' (permissions?)\n", path);
        /* non-fatal */
        return 0;
    }

    if (type == FTW_SL && opts.nodereference == 1) {
        if (opts.group_too == 1) {
            if (lchown(path, own_buf->pw_uid, grp_buf->gr_gid) != 0) {
                perror("chown failed");
            }
        } else {
            if (lchown(path, own_buf->pw_uid, -1) != 0) {
                perror("chown failed");
            }
        }
    } else {
        if (opts.group_too == 1) {
            if (chown(path, own_buf->pw_uid, grp_buf->gr_gid) != 0) {
                perror("chown failed");
            }
        } else {
            if (chown(path, own_buf->pw_uid, -1) != 0) {
                perror("chown failed");
            }
        }
    }

    if (opts.verbose == 1) {
        printf("changed file ownership of `%s' to `%s'\n", path, to_own);
        if (opts.group_too == 1) {
            printf("changed group ownership of `%s' to `%s'\n", path, to_grp);
        }
    }

    return 0;
}

int main(int argc, char *argv[]) {
    int opt;

    struct option longopts[] = {
        {"help",           0, NULL, 'h'},
        {"version",        0, NULL, 'V'},
        {"recursive",      0, NULL, 'R'},
        {"verbose",        0, NULL, 'v'},
        {"no-dereference", 0, NULL, 'd'},
        {0,0,0,0}
    };

    while ((opt = getopt_long(argc, argv, "VhRvd", longopts, NULL)) != -1) {
        switch(opt) {
            case 'V':
                printf("%s (%s) version %s\n", APPNAME, APPSUITE, APPVERSION);
                printf("%s compiled on %s at %s\n", basename(__FILE__), __DATE__, __TIME__);
                exit(EXIT_SUCCESS);
            case 'h':
                show_help();
                exit(EXIT_SUCCESS);
            case 'R':
                opts.recursive = 1;
                break;
            case 'v':
                opts.verbose = 1;
                break;
            case 'd':
                opts.nodereference = 1;
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
                break;
        }
    }

    if (strchr(argv[optind], 58) != NULL) {
        /* user:group */
        po = strtok(argv[optind], ":");
        pg = strtok(NULL, ":");
        strncpy(to_own, po, FILEMAX);
        strncpy(to_grp, pg, FILEMAX);

        opts.group_too = 1;
    } else {
        /* only specified user (new owner) */
        strncpy(to_own, argv[optind], FILEMAX);
    }
    optind++;

    own_buf = getpwnam(to_own);
    if (own_buf == NULL) {
        printf("Could not resolve user name: %s\n", to_own);
        exit(EXIT_FAILURE);
    }

    grp_buf = getgrnam(to_grp);
    if (opts.group_too == 1 && grp_buf == NULL) {
        printf("Could not resolve group name: %s\n", to_grp);
        exit(EXIT_FAILURE);
    }

    if (opts.recursive == 1) {
        //struct stat stat_buf;
        //struct FTW ftw_buf;

        if (opts.nodereference == 1) {
            if (nftw(argv[optind], chown_recurse, 10, FTW_PHYS) != 0) {
                perror("chgrp");
            }
        } else {
            if (nftw(argv[optind], chown_recurse, 10, 0) != 0) {
                perror("chgrp");
            }
        }
        return EXIT_SUCCESS;
    }

    while (optind < argc) {
        if (opts.nodereference == 1) {
            if (opts.group_too == 1) {
                if (lchown(argv[optind], own_buf->pw_uid, grp_buf->gr_gid) != 0) {
                perror("chown failed");
                }
            } else {
                if (lchown(argv[optind], own_buf->pw_uid, -1) != 0) {
                perror("chown failed");
                }
            }

        } else {
            if (opts.group_too == 1) {
                if (chown(argv[optind], own_buf->pw_uid, grp_buf->gr_gid) != 0) {
                perror("chown failed");
                }
            } else {
                if (chown(argv[optind], own_buf->pw_uid, -1) != 0) {
                perror("chown failed");
                }
            }
        }

        if (opts.verbose == 1) {
            printf("changed file ownership of `%s' to `%s'\n", argv[optind], to_own);
            if (opts.group_too == 1) {
                printf("changed group ownership of `%s' to `%s'\n", argv[optind], to_grp);
            }
        }

        optind++;
    }

    return EXIT_SUCCESS;
}
