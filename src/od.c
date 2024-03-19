/***************************************************************************
 *   od.c - object dump                                                    *
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


#include "common.h"
#define APPNAME "od"

void show_help(void) {
    printf("Usage: %s [OPTION]...\n\n\
Options:\n\
    -h, --help\t\tdisplay this help\n\
    -V, --version\tdisplay version information\n\n\
Report bugs to <bulliver@gmail.com>\n", APPNAME);
}

int read_file_dec(char *filename[]) {
    FILE *fp;
    char c;

    if ((fp = fopen(*filename, "r")) == NULL ) {
        f_error(*filename, "Cannot open ");
    }

    int offset = 0;

    printf("%08d ", offset);
    while (( c = getc(fp)) != EOF ) {
        printf("%03d", (unsigned)c );
        offset++;

        if (offset % 2 == 0) {
            printf(" ");
        }

        if (offset % 16 == 0) {
            printf("\n");
            printf("%08d ", offset);
        }
    }
    printf("\n");
    printf("%08d\n", offset);

    return 0;
}

int read_file_hex(char *filename[]) {
    FILE *fp;
    char c;

    if ((fp = fopen(*filename, "r")) == NULL ) {
        f_error(*filename, "Cannot open ");
    }

    int offset = 0;

    printf("0x%08x ", offset);
    while (( c = getc(fp)) != EOF ) {
        printf("%02x", (unsigned)c );
        offset++;

        if (offset % 2 == 0) {
            printf(" ");
        }

        if (offset % 16 == 0) {
            printf("\n");
            printf("0x%08x ", offset);
        }
    }
    printf("\n");
    printf("0x%08x\n", offset);

    return 0;
}

int read_file_oct(char *filename[]) {
    FILE *fp;
    char c;

    if ((fp = fopen(*filename, "r")) == NULL ) {
        f_error(*filename, "Cannot open ");
    }

    int offset = 0;

    printf("0%08o ", offset);
    while (( c = getc(fp)) != EOF ) {
        printf("%03o", (unsigned)c );
        offset++;
        printf(" ");

        if (offset % 16 == 0) {
            printf("\n");
            printf("0%08o ", offset);
        }
    }
    printf("\n");
    printf("0%08o\n", offset);

    return 0;
}

int main(int argc, char *argv[]) {
    int opt;

    struct option longopts[] = {
        {"hex", 0, NULL, 'x'},
        {"octal", 0, NULL, 'o'},
        {"decimal", 0, NULL, 'd'},
        {"skip-bytes", 0, NULL, 'j'},
        {"read-bytes", 0, NULL, 'N'},
        {"help", 0, NULL, 'h'},
        {"version", 0, NULL, 'V'},
        {0,0,0,0}
    };

    int format = 0x0;

    while ((opt = getopt_long(argc, argv, "VhxodjN", longopts, NULL)) != -1) {
        switch(opt) {
            case 'x':
                format = 0x1;
                break;
            case 'd':
                format = 0x2;
                break;
            case 'o':
                break;
            case 'V':
                printf("%s (%s) version %s\n", APPNAME, APPSUITE, APPVERSION);
                printf("%s compiled on %s at %s\n", basename(__FILE__), __DATE__, __TIME__);
                exit(EXIT_SUCCESS);
                break;
            case 'h':
                show_help();
                exit(EXIT_SUCCESS);
                break;
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


    if (format == 0x1) {
        read_file_hex(&argv[optind]);
    }
    else if (format == 0x2) {
        read_file_dec(&argv[optind]);
    } else {
        read_file_oct(&argv[optind]);
    }

    return EXIT_SUCCESS;
}
