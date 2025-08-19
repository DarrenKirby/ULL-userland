/***************************************************************************
 *   chgrp.c - change group ownership of file                              *
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

#include "common.h"
#include <ftw.h>

const char *APPNAME = "chgrp";

struct group *grp_buf;
char to_grp[FILEMAX];

struct opt_struct {
    int no_dereference;
    int recursive;
    int verbose;
} opts;


static void show_help(void) {
    printf("Usage: %s [OPTION] group FILE [FILE]...\n\n\
Change group ownership of file(s)\n\n\
Options:\n\
    -R, --recursive\t\tchange group of files recursively\n\
    -v, --verbose\t\toutput a diagnostic for every file processed\n\
    -d, --no-dereference\toperate on symbolic links rather than their targets\n\
    -h, --help\t\t\tdisplay this help\n\
    -V, --version\t\tdisplay version information\n\n\
Report bugs to <bulliver@gmail.com>\n", APPNAME);
}

int chgrp_recurse(const char *path, const struct stat *stat_ptr, const int type, struct FTW *p_fwt) {
    if (type == FTW_NS) {
        fprintf(stderr, "stat failed on `%s' (permissions?)\n", path);
        return EXIT_FAILURE;
        /* non-fatal */
    }

    if (type == FTW_SL && opts.no_dereference == 1) {
        if (lchown(path, -1, grp_buf->gr_gid) != 0) {
            fprintf(stderr, "lchown failed on `%s'\n", path);
        }
    } else {
        if (chown(path, -1, grp_buf->gr_gid) != 0) {
            fprintf(stderr, "chown failed on `%s'\n", path);
        }
    }

    if (opts.verbose == 1) {
        printf("Changed group ownership of `%s' to `%s'\n", path, to_grp);
    }
    return EXIT_SUCCESS;
}

int main(const int argc, char *argv[]) {
    int opt;

    const struct option long_opts[] = {
        {"help",           0, NULL, 'h'},
        {"version",        0, NULL, 'V'},
        {"recursive",      0, NULL, 'R'},
        {"verbose",        0, NULL, 'v'},
        {"no-dereference", 0, NULL, 'd'},
        {NULL,0,NULL,0}
    };

    while ((opt = getopt_long(argc, argv, "VhRvd", long_opts, NULL)) != -1) {
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
            case 'R':
                opts.recursive = 1;
                break;
            case 'v':
                opts.verbose = 1;
                break;
            case 'd':
                opts.no_dereference = 1;
                break;
            default:
                show_help();
                exit(EXIT_FAILURE);
                break;
        }
    }

    strncpy(to_grp, argv[optind], FILEMAX);

    grp_buf = getgrnam(argv[optind]);
    if (grp_buf == NULL) {
        printf("Could not resolve group name: %s", argv[optind]);
        exit(EXIT_FAILURE);
    }
    optind++;

    if (opts.recursive == 1) {
        /* Status of chgrp calls checked within chgrp_recursive()
         * A failure of any particular file will be noted, but we
         * will continue to run... */
        if (opts.no_dereference == 1) {
            nftw(argv[optind], chgrp_recurse, 10, FTW_PHYS);
        } else {
            nftw(argv[optind], chgrp_recurse, 10, 0);
        }
        return EXIT_SUCCESS;
    }

    while (optind < argc) {
        if (opts.no_dereference == 1) {
            if (lchown(argv[optind], -1, grp_buf->gr_gid) != 0) {
                fprintf(stderr, "lchown failed on `%s'\n", argv[optind]);
            }
        }else {
            if (chown(argv[optind], -1, grp_buf->gr_gid) != 0) {
                fprintf(stderr, "chown failed on `%s'\n", argv[optind]);
            }
        }

        if (opts.verbose == 1) {
            printf("Changed group ownership of `%s' to `%s'\n", argv[optind], to_grp);
        }
        optind++;
    }
    return EXIT_SUCCESS;
}
