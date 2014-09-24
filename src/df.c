/***************************************************************************
 *   df.c - report file system disk space usage                            *
 *                                                                         *
 *   Copyright (C) 2014 by Darren Kirby                                    *
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

#define APPNAME "df"
#include "mount.h"
#include "common.h"

struct packed_flags {
    unsigned int b : 1; /* block size (k, m, g, t, p, e, z, y (*1024) or K, M, G..etc (*1000)) */
    unsigned int r : 1; /* human readable (*1024) */
    unsigned int H : 1; /* human readable si (*1000) */
    unsigned int i : 1; /* inodes */
    unsigned int T : 1; /* FS type */
    unsigned int t : 1; /* total */
    unsigned int a : 1; /* include dummy file systems */
};

struct packed_flags flags = {0,0,0,0,0,0};
int fmt = 0;
char get_option_char;

void show_help(void) {
    printf("Usage: %s [OPTION]...\n\n\
Options:\n\
    -h, --help\t\tdisplay this help\n\
    -V, --version\tdisplay version information\n\n\
Report bugs to <bulliver@gmail.com>\n", APPNAME);
}

int main(int argc, char *argv[]) {
    int opt;

    struct option longopts[] = {
        {"help", 0, NULL, 'h'},
        {"version", 0, NULL, 'V'},
        {"block-size", required_argument, NULL, 'b'},
        {"human-readable", 0, NULL, 'r'},
        {"human-readable-si", 0, NULL, 'H'},
        {"inodes", 0, NULL, 'i'},
        {"kilobytes", 0, NULL, 'k'},
        {"megabytes", 0, NULL, 'm'},
        {"gigabytes", 0, NULL, 'g'},
        {"sync", 0, NULL, 's'},
        {"fs-type", 0, NULL, 'T'},
        {"total", 0, NULL, 't'},
        {"all", 0, NULL, 'a'},
        {0,0,0,0}
    };

    while ((opt = getopt_long(argc, argv, "Vhb:rHikmgsTta", longopts, NULL)) != -1) {
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
            case 'b': 
                get_option_char = optarg[0];
                switch(get_option_char) {
                    case 'k': fmt = 1; break;
                    case 'm': fmt = 2; break;
                    case 'g': fmt = 3; break;
                    case 't': fmt = 4; break;
                    case 'p': fmt = 5; break;
                    case 'e': fmt = 6; break;
                    case 'z': fmt = 7; break;
                    case 'y': fmt = 8; break;
                    case 'K': fmt = 9; break;
                    case 'M': fmt = 10; break;
                    case 'G': fmt = 11; break;
                    case 'T': fmt = 12; break;
                    case 'P': fmt = 13; break;
                    case 'E': fmt = 14; break;
                    case 'Z': fmt = 15; break;
                    case 'Y': fmt = 16; break;
                }
            case 'r':
                flags.r = 1;
                break;
            case 'H':
                flags.H = 1;
                break;
            case 'i':
                flags.i = 1;
                break;
            case 'k':
                fmt = 1;
                break;
            case 'm':
                fmt = 2;
            case 'g':
                fmt = 3;
                break;
            case 's':
                sync();
                break;
            case 'T':
                flags.T = 1;
                break;
            case 't':
                flags.t = 1;
            case 'a':
                flags.a = 1;
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
    struct statfs_full *foo;
    int n_mounts;
#if defined (__linux__)
    if (argc == optind) /* display all mounted file systems */
        n_mounts = getfsstat_linux(foo, 4048);
    
    printf("Found %i mounted file systems\n", n_mounts);
    printf("sizeof foo: %lu\n", sizeof(foo));
    return EXIT_SUCCESS;
#else
    if (argc == optind) /* display all mounted file systems */
        n_mounts = getfsstat(foo, 500000, NULL);
    
    printf("Found %i mounted file systems\n", n_mounts);
    return EXIT_SUCCESS;
#endif

}

