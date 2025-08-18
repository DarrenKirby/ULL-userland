/***************************************************************************
 *   cat.c - concatenate files and print on the standard output            *
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


#include "common.h"
const char *APPNAME = "cat";

static void show_help(void) {
    printf("Usage: %s [OPTION]... [FILE]...\n\
Concatenate FILE(s), or standard input to standard output.\n\n\
Options:\n\
    -n, --number\tnumber lines\n\
    -u, --unbuffered\tsets stdout to be unbuffered\n\
    -h, --help\t\tdisplay this help\n\
    -V, --version\tdisplay version information\n\n\
Report bugs to <bulliver@gmail.com>\n", APPNAME);
}

/* flag which tells whether we are numbering lines */
int number_lines = 0;

static void cat_stdin(const int unbuffered) {
    int c;
    int line_number = 1;

    if (unbuffered == 1) {
        setvbuf(stdout, NULL, _IONBF, 1);
    }

    printf("%6u\t", line_number);
    line_number++;
    while (( c = getc(stdin)) != EOF ) {
        if  (c == 10 && number_lines == 1) {
            putc(c, stdout);
            printf("%6u\t", line_number);
            line_number++;
        } else {
            putc(c, stdout);
        }
    }
    printf("\n");
    exit(EXIT_SUCCESS);
}

static void cat_file(char *filename, int line_number) {
    int c;
    FILE *name;

    if ((name = fopen(filename, "r")) == NULL ) {
        fprintf(stderr, "cat: cannot open file %s\n", filename);
    }

    while ((c = getc(name)) != EOF ) {
        if  (c == 10 && number_lines == 1) {
            putc(c, stdout);
            printf("%6u\t", line_number);
            line_number++;
        } else {
            putc(c, stdout);
        }
    }
    fclose(name);
}

int main(const int argc, char *argv[]) {
    int opt;
    int line_number = 1;
    int unbuffered = 0;

    const struct option long_opts[] = {
        {"help", 0, NULL, 'h'},
        {"version", 0, NULL, 'V'},
        {"number", 0, NULL, 'n'},
        {"unbuffered", 0, NULL, 'u'},
        {NULL,0,NULL,0}
    };

    while ((opt = getopt_long(argc, argv, "Vhnu", long_opts, NULL)) != -1) {
        switch(opt) {
            case 'n':
                number_lines = 1;
                break;
            case 'u':
                unbuffered = 1;
                break;
            case 'V':
                printf("%s (%s) version %s\n", APPNAME, APPSUITE, APPVERSION);
                printf("%s compiled on %s at %s\n",
                       strrchr(__FILE__, '/') ? strrchr(__FILE__, '/') + 1 : __FILE__,
                       __DATE__, __TIME__);
                exit(EXIT_SUCCESS);
            case 'h':
                show_help();
                exit(EXIT_SUCCESS);
            default:
                show_help();
                exit(EXIT_FAILURE);
        }
    }

    if (argc == optind || strcmp(argv[optind], "-") == 0) {  /* no file arguments */
        cat_stdin(unbuffered);
    }

    if (number_lines == 1) {
        printf("%6u\t", line_number);
        line_number++;
    }

    while (optind < argc) {
        cat_file(argv[optind], line_number);
        optind++;
    }
    printf("\n");
    return EXIT_SUCCESS;
}
