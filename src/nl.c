/***************************************************************************
 *   nl.c - print file with line numbers                                   *
 *                                                                         *
 *   Copyright (C) 2014 - 2026 by Darren Kirby                             *
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
#include <math.h>

#include "common.h"

#define LINE_SIZE 1024
#define BUFF_SIZE 4096

static const char *APP_NAME =  "nl";


static void show_help() {
    printf("Usage: %s [OPTION]...\n\n\
Options:\n\
    -h, --help\t\tdisplay this help\n\
    -V, --version\tdisplay version information\n\n\
Report bugs to <bulliver@gmail.com>\n", APP_NAME);
}

static int count_lines(FILE *fp) {
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

static void nl_stdin(const int unbuffered) {
    int c;
    int line_number = 1;

    if (unbuffered == 1) {
        setvbuf(stdout, nullptr, _IONBF, 1);
    }

    printf("%4i | ", line_number);
    line_number++;
    while (( c = getc(stdin)) != EOF ) {
        if  (c == 10) {
            putc(c, stdout);
            /*
             * we cannot precalculate number of lines in stdin,
             * so our width is just fixed and arbitrary. Surely,
             * noone would cat a file with more than 9999 lines
             * into the terminal?
             */
            printf("%4i | ", line_number);
            line_number++;
        } else {
            putc(c, stdout);
        }
    }
    printf("\n");
    exit(EXIT_SUCCESS);
}

int main(const int argc, char *argv[])
{
    int opt;

    const struct option longopts[] = {
        {.name = "help",    .has_arg = no_argument, .flag = nullptr, .val = 'h'},
        {.name = "version", .has_arg = no_argument, .flag = nullptr, .val = 'V'},
        {.name = "stats",   .has_arg = no_argument, .flag = nullptr, .val = 's'},
        {.name = nullptr,   .has_arg = no_argument, .flag = nullptr, .val = 0}
    };

    while ((opt = getopt_long(argc, argv, "Vhs", longopts, nullptr)) != -1) {
        switch(opt) {
            case 's':
                printf("Stats coming soon\n");
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


    char buf[LINE_SIZE];
    int lineno = 1;
    constexpr int unbuffered = 0;

    if (argc == 1)
        nl_stdin(unbuffered);

    FILE *fd = fopen(argv[1], "r");

    const int width = count_lines(fd);
    rewind(fd);

    while (fgets(buf, LINE_SIZE, fd) != NULL) {
        printf("%*i | %s", width, lineno, buf);
        lineno++;
    }


    return EXIT_SUCCESS;
}
