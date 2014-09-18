/***************************************************************************
 *   cat.c - concatenate files and print on the standard output            *
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

#define APPNAME "cat"
#include "common.h"

void show_help(void) {
    printf("Usage: %s [OPTION]... [FILE]...\n \
Concatenate FILE(s), or standard input to standard output.\n\n \
Options:\n\
    -n, --number\t\tnumber lines\n \
    -h, --help\t\tdisplay this help\n \
    -V, --version\tdisplay version information\n\n \
Report bugs to <bulliver@gmail.com>\n", APPNAME);
}

int number_lines = 0;
FILE *name;
char c;

void cat_stdin(int line_number) {
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

int cat_file(char *filename, unsigned int line_number) {
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

    struct option longopts[] = {
        {"help", 0, NULL, 'h'},
        {"version", 0, NULL, 'V'},
        {"number", 0, NULL, 'n'},
        {0,0,0,0}
    };

    while ((opt = getopt_long(argc, argv, "Vhn", longopts, NULL)) != -1) {
        switch(opt) {
            case 'n':
                number_lines = 1;
                break;
            case 'V':
                printf("%s (%s) version %s\n", APPNAME, APPSUITE, APPVERSION);
                exit(EXIT_SUCCESS);
                break;
            case 'h':
                show_help();
                exit(EXIT_SUCCESS);
                break;
            case ':':
                printf("option '%c' needs an argument\n", optopt);
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
    
    if (argc == optind) {        /* no file arguments */
        cat_stdin(line_number);
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

