/***************************************************************************
 *   cat.c - concatenate files and print on the standard output            *
 *                                                                         *
 *   Copyright (C) 2014-2025 by Darren Kirby                               *
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

#include <getopt.h>

#include "common.h"


static const char *APP_NAME = "cat";

static void show_help() {
    printf("Usage: %s [OPTION]... [FILE]...\n\
Concatenate FILE(s), or standard input to standard output.\n\n\
Options:\n\
    -n, --number\t number lines\n\
    -u, --unbuffered\t sets stdout to be unbuffered\n\
    -h, --help\t\t display this help\n\
    -V, --version\t display version information\n\n\
Report bugs to <darren@dragonbyte.ca>\n", APP_NAME);
}

/* flag which tells whether we are numbering lines */
static bool number_lines = false;

static void cat_stdin(const int unbuffered) {
    int c;
    int line_number = 1;

    if (unbuffered) {
        setvbuf(stdout, nullptr, _IONBF, 1);
    }

    while (( c = getc(stdin)) != EOF ) {
        if  (c == 10 && number_lines == 1) {
            putc(c, stdout);
            printf("%6u\t", line_number++);
        } else {
            putc(c, stdout);
        }
    }
    printf("\n");
    exit(EXIT_SUCCESS);
}

static void cat_file(char *filename, uint32_t *line_number) {
    FILE *fd;

    if ((fd = fopen(filename, "r")) == NULL ) {
        fprintf(stderr, "cannot open file %s: %s\n", filename, strerror(errno));
        exit(EXIT_FAILURE);
    }

    char *line = nullptr;
    size_t line_cap = 0;
    ssize_t line_len;
    while ((line_len = getline(&line, &line_cap, fd)) > 0) {
        if  (number_lines) {
            printf("%6u\t%s", *line_number, line);
            (*line_number)++;
        } else {
            fwrite(line, line_len, 1, stdout);
        }
    }
    fclose(fd);
}

int main(const int argc, char *argv[]) {
    uint32_t line_number = 1;
    bool unbuffered = 0;

    const struct option long_opts[] = {
        {"help", 0, nullptr, 'h'},
        {"version", 0, nullptr, 'V'},
        {"number", 0, nullptr, 'n'},
        {"unbuffered", 0, nullptr, 'u'},
        {nullptr,0,nullptr,0}
    };

    int opt;
    while ((opt = getopt_long(argc, argv, "Vhnu", long_opts, nullptr)) != -1) {
        switch(opt) {
            case 'n':
                number_lines = true;
                break;
            case 'u':
                unbuffered = true;
                break;
            case 'V':
                printf("%s (%s) version %s\n", APP_NAME, APP_SUITE, APP_VERSION);
                printf("%s compiled on %s at %s\n",
                       strrchr(__FILE__, '/') ? strrchr(__FILE__, '/') + 1 : __FILE__,
                       __DATE__, __TIME__);
                return EXIT_SUCCESS;
            case 'h':
                show_help();
                return EXIT_SUCCESS;
            default:
                show_help();
                return EXIT_FAILURE;
        }
    }

    if (argc == optind || strcmp(argv[optind], "-") == 0) {  /* no file arguments */
        cat_stdin(unbuffered);
        return EXIT_SUCCESS;
    }

    while (optind < argc) {
        cat_file(argv[optind++], &line_number);
    }
    printf("\n");
    return EXIT_SUCCESS;
}
