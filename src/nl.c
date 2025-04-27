/***************************************************************************
 *   nl.c - print file with line numbers                                   *
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


#include <math.h>
#include "common.h"
const char *APPNAME =  "nl";

//struct optstruct {
//} opts;

static void show_help(void) {
    printf("Usage: %s [OPTION]...\n\n\
Options:\n\
    -h, --help\t\tdisplay this help\n\
    -V, --version\tdisplay version information\n\n\
Report bugs to <bulliver@gmail.com>\n", APPNAME);
}

#define LINE_SIZE 1024
#define BUFF_SIZE 4096


int count_lines(FILE *fp) {
    char buf[BUFF_SIZE];
    size_t bytes_read;
    int lines = 0;

    while ((bytes_read = fread(buf, 1, BUFF_SIZE, fp)) > 0) {
        for (size_t i = 0; i < bytes_read; i++) {
            if (buf[i] == '\n') {
                lines++;
            }
        }
    }

    if (lines == 0) {
        return 2;
    }

    return (int)log10(lines) + 2;
}

int main(int argc, char *argv[]) {
    int opt;

    struct option longopts[] = {
        {"help", 0, NULL, 'h'},
        {"version", 0, NULL, 'V'},
        {"stats", 0, NULL, 's'},
        {0,0,0,0}
    };

    while ((opt = getopt_long(argc, argv, "Vhs", longopts, NULL)) != -1) {
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

    FILE *fd;
    char buf[LINE_SIZE];
    int lineno = 1;
    int width;

    if (argc >= 2)
        fd = fopen(argv[1], "r");
    else {
        printf("Does not work with stdin yet. Please pass a filename\n");
        return -1;
    }


    width = count_lines(fd);
    rewind(fd);

    while (fgets(buf, LINE_SIZE, fd) != NULL) {
        printf("%*i | %s", width, lineno, buf);
        lineno++;
    }


    return EXIT_SUCCESS;
}
