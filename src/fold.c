/***************************************************************************
 *   fold - wrap each input line to fit in specified width                 *
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
#include <inttypes.h>
#include <stdlib.h>

#include "common.h"


static const char *APP_NAME = "fold";

static struct {
    bool chars;
    bool bytes;
    uint16_t width;
} opts = {
    .chars = true,
    .bytes = false,
    .width = 80 };

static void show_help()
{
    printf("Usage: %s [OPTION]...\n\n\
Options:\n\
    -h, --help\t\t display this help\n\
    -V, --version\t display version information\n\n\
    -c, --chars\tcount as characters\n\
    -b, --bytes\tcount as bytes\n\
    -w, --width=N\twrap lines at N cols (default 80)\n\
Report bugs to <darren@dragonbyte.ca>\n", APP_NAME);
}

static FILE* open_file(char *file) {
    errno = 0;
    FILE *fp = fopen(file, "r");
    if (!fp) {
        fprintf(stderr, "%s: unable to open %s: %s", APP_NAME, file, strerror(errno));
        exit(EXIT_FAILURE);
    }
    return fp;
}

static char* alloc_char_buffer() {
    char *buf = malloc(opts.width + 1);
    if (!buf) {
        fprintf(stderr, "%s: unable to allocate memory!\n", APP_NAME);
        exit(EXIT_FAILURE);
    }
    return buf;
}

static uint8_t* alloc_byte_buffer() {
    uint8_t *buf = malloc(opts.width);
    if (!buf) {
        fprintf(stderr, "%s: unable to allocate memory!\n", APP_NAME);
        exit(EXIT_FAILURE);
    }
    return buf;
}

static void fold_on_bytes(char *file) {
    uint8_t *buf = alloc_byte_buffer();
    FILE *fp;

    if (*file == 0x2D) {
        fp = stdin;
    } else {
        fp = open_file(file);
    }

    int c = '\0';
    while (c != EOF) {
        for (int i = 0; i < opts.width; i++) {
            c = fgetc(fp);
            if (c == EOF) break;
            putc(c, stdout);
            if (c == 10) break;
        }
        if (c != 10) {
            putc(10, stdout);
        }
    }

    fclose(fp);
    free(buf);
}

static void fold_on_chars(char *file) {
    char *buf = alloc_char_buffer();
    FILE *fp;

    if (*file == 0x2D) {
        fp = stdin;
    } else {
        fp = open_file(file);
    }

    while (fgets(buf, opts.width + 1, fp) != NULL) {
        printf("%s%s", buf, strchr(buf, '\n') != NULL ? "" : "\n");
    }

    fclose(fp);
    free(buf);
}

int main(const int argc, char *argv[])
{
    const struct option long_opts[] = {
        {.name = "help",    .has_arg = no_argument,       .flag = nullptr, .val = 'h'},
        {.name = "version", .has_arg = no_argument,       .flag = nullptr, .val = 'V'},
        {.name = "width",   .has_arg = required_argument, .flag = nullptr, .val = 'w'},
        {.name = "chars",   .has_arg = no_argument,       .flag = nullptr, .val = 'c'},
        {.name = "bytes",   .has_arg = no_argument,       .flag = nullptr, .val = 'b'},
        {.name = nullptr,   .has_arg = no_argument,       .flag = nullptr, .val = 0}
    };

    int opt;
    while ((opt = getopt_long(argc, argv, "Vhw:cb", long_opts, nullptr)) != -1) {
        switch(opt) {
            case 'V':
                printf("%s (%s) version %s\n", APP_NAME, APP_SUITE, APP_VERSION);
                printf("%s compiled on %s at %s\n",
                       strrchr(__FILE__, '/') ? strrchr(__FILE__, '/') + 1 : __FILE__,
                       __DATE__, __TIME__);
                return EXIT_SUCCESS;
            case 'h':
                show_help();
                return EXIT_SUCCESS;
            case 'c':
                opts.chars = true;
                opts.bytes = false;
                break;
            case 'b':
                opts.bytes = true;
                opts.chars = false;
                break;
            case 'w':
                opts.width = (uint8_t)parse_numeric_arg(optarg, 1, 255, APP_NAME);
                break;
            default:
                show_help();
                return EXIT_FAILURE;
        }
    }

    if (argc == optind || strcmp(argv[optind], "-") == 0) {  /* no file arguments */
        if (opts.chars) {
            fold_on_chars("-");
        } else {
            fold_on_bytes("-");
        }
        return EXIT_SUCCESS;
    }

    while (optind < argc) {
        if (opts.chars) {
            fold_on_chars(argv[optind++]);
        } else {
            fold_on_bytes(argv[optind++]);
        }
    }

    return EXIT_SUCCESS;
}
