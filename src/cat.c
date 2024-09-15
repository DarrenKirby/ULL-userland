/***************************************************************************
 *   cat.c - concatenate files and print on the standard output            *
 *                                                                         *
 *   Copyright (C) 2014-2024 by Darren Kirby                               *
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

#define APPNAME "cat"

static void show_help(void) {
    printf("Usage: %s [OPTION]... [FILE]...\n \
Concatenate FILE(s), or standard input to standard output.\n\n \
Options:\n\
    -n, --number\tnumber lines\n \
   -u, --unbuffered\tsets stdout to be unbuffered\n \
   -h, --help\t\tdisplay this help\n \
   -V, --version\tdisplay version information\n\n \
Report bugs to <bulliver@gmail.com>\n", APPNAME);
}


int number_lines = 0;
FILE *name;
signed char c;

static void cat_stdin(int line_number, int unbuffered) {
    if (unbuffered == 1) {
        setvbuf(stdout, NULL, _IONBF, 1);
    }

    while (( c = getc(stdin)) != EOF ) {
        if  (c == 10 && number_lines == 1) {
            putc(c, stdout);
            printf("%6u\t", line_number);
            line_number++;
        } else {
            putc(c, stdout);
        }
    }
    fclose(name);
    exit(EXIT_SUCCESS);
}

static int cat_file(char *filename, unsigned int line_number) {
    if ( (name = fopen(filename, "r")) == NULL ) {
        f_error(filename, NULL);
    }

    while (( c = getc(name)) != EOF ) {
        if  (c == 10 && number_lines == 1) {
            putc(c, stdout);
            printf("%6u\t", line_number);
            line_number++;
        } else {
            putc(c, stdout);
        }
    }
    fclose(name);
    return line_number;
}

int main(int argc, char *argv[]) {
    int opt;
    unsigned int line_number = 1;
    int unbuffered = 0;

    struct option longopts[] = {
        {"help", 0, NULL, 'h'},
        {"version", 0, NULL, 'V'},
        {"number", 0, NULL, 'n'},
        {"unbuffered", 0, NULL, 'u'},
        {0,0,0,0}
    };

    while ((opt = getopt_long(argc, argv, "Vhnu", longopts, NULL)) != -1) {
        switch(opt) {
            case 'n':
                number_lines = 1;
                break;
            case 'u':
                unbuffered = 1;
                break;
            case 'V':
                printf("%s (%s) version %s\n", APPNAME, APPSUITE, APPVERSION);
                printf("%s compiled on %s at %s\n", basename(__FILE__), __DATE__, __TIME__);
                exit(EXIT_SUCCESS);
            case 'h':
                show_help();
                exit(EXIT_SUCCESS);
            case ':':
                printf("option '%c' needs an argument\n", optopt);
                exit(EXIT_FAILURE);
            case '?':
                 /* getopt_long prints own error message */
                exit(EXIT_FAILURE);
            default:
                show_help();
                exit(EXIT_FAILURE);
        }
    }

    if (argc == optind) {        /* no file arguments */
        cat_stdin(line_number, unbuffered);
    }

    if (number_lines == 1) {
        printf("%6u\t", line_number);
        line_number++;
    }

    while (optind < argc) {
        line_number = cat_file(argv[optind], line_number);
        optind++;
    }
    printf("\n");
    return EXIT_SUCCESS;
}
