/***************************************************************************
 *   rm.c - remove files or directories                                    *
 *                                                                         *
 *   Copyright (C) 2014-2025 by Darren Kirby                               *
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

#include <unistd.h>

#include "common.h"

const char *APPNAME = "rm";

static void showHelp(void) {
    printf("Usage: %s [OPTION]... [FILE]...\n\n\
    Remove (unlink) FILE(s).\n\n\
Options:\n\
    -f, --force\t\t not implemented yet\n\
    -r, -R, --recursive  not implemented yet\n\
    -i, --interactive\t prompt before every removal\n\
    -v, --verbose\t explain what is being done\n\
    -h, --help\t\t display this help and exit\n\
    -V, --version\t display version information\n\n\
Report bugs to <bulliver@gmail.com>\n", APPNAME);
}

struct opt_struct {
    int verbose;
    int recursive;
    int force;
    int interactive;
} opts;

int rm(char *file) {
    const int es = unlink(file);
    if (es != 0) {
        if (opts.force == 1 && errno == 2) { /* --force ignores non-existing files */
            return EXIT_SUCCESS;
        }
        fprintf(stderr, "rm: %s: %s\n", file, strerror(errno));
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}

int main(const int argc, char *argv[]) {
    int opt;

    const struct option long_opts[] = {
        {"help", 0, NULL, 'h'},
        {"force", 0, NULL, 'f'},
        {"interactive", 0, NULL, 'i'},
        {"recursive", 0, NULL, 'R'},
        {"verbose", 0, NULL, 'v'},
        {"version", 0, NULL, 'V'},
        {NULL,0,NULL,0}
    };

    while ((opt = getopt_long(argc, argv, "hrvVRif", long_opts, NULL)) != -1) {
        switch(opt) {
            case 'r':
            case 'R':
                printf("Option 'recursive' not implemented yet\n");
                opts.recursive = 1;
                break;
            case 'i':
                opts.interactive = 1;
                break;
            case 'v':
                opts.verbose = 1;
                break;
            case 'f':
                opts.force = 1;
                break;
            case 'h':
                showHelp();
                exit(EXIT_SUCCESS);
            case 'V':
                printf("%s (%s) %s\n", APPNAME, APPSUITE, APPVERSION);
                printf("%s compiled on %s at %s\n",
                       strrchr(__FILE__, '/') ? strrchr(__FILE__, '/') + 1 : __FILE__,
                       __DATE__, __TIME__);
                exit(EXIT_SUCCESS);
            default:
                showHelp();
                exit(EXIT_FAILURE);
        }
    }

    /* TODO: implement recursive */
    if (opts.interactive == 1) {
        int response;
        for (; optind < argc; optind++) {

            printf("rm: remove %s ('y' or 'n')? ", argv[optind]);
            do {
                response = getchar();
            }
            while (response == '\n');

            if (response == 'y' || response == 'Y') {
                const int es = rm(argv[optind]);
                if (es != EXIT_SUCCESS) {
                    printf("failed to remove %s\n", argv[optind]);
                    continue;
                }
                if (opts.verbose == 1) {
                    printf("removed '%s'\n", argv[optind]);
                }
            }
        }
    } else {
        for (; optind < argc; optind++) {
            const int es = rm(argv[optind]);
            if (es != EXIT_SUCCESS) {
                printf("failed to remove %s\n", argv[optind]);
                continue;
            }
            if (opts.verbose == 1) {
                printf("removed '%s'\n", argv[optind]);
            }
        }
    }

    return EXIT_SUCCESS;
}
