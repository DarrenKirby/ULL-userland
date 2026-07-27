/***************************************************************************
 *   tail.c - print last n lines or bytes of file                          *
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
#include <stdint.h>
#include <sys/fcntl.h>

#include "common.h"

#define MAX_LINE_LENGTH 2048


static const char *APP_NAME = "tail";

static struct {
    bool quiet:1;
    bool verbose:1;
    bool bytes:1;
    bool lines:1;
} opts = {
    .quiet = false,
    .verbose = false,
    .bytes = false,
    .lines = true };

static void show_help()
{
    printf("Usage: %s [OPTION]... FILE [FILE...]\n\n\
Print last N lines or bytes of file\n\n\
Options:\n\
    -n, --lines=N\t\t print first N lines\n\
    -b, --bytes=N\t\t print first N bytes instead of lines\n\
    -v, --verbose\t\t always print file header(s)\n\
    -q, --quiet\t\t never print file header(s)\n\
    -h, --help\t\t display this help\n\
    -V, --version\t display version information\n\n\
Report bugs to <darren@dragonbyte.ca>\n", APP_NAME);
}

static int tail_bytes(char *filename, const uint32_t n_bytes)
{
    if (opts.verbose) {
        printf("==> %s%s%s <==\n", ANSI_BLUE_B, filename, ANSI_RESET);
    }

    FILE *fd = fopen(filename, "r");
    if (!fd) {
        fprintf(stderr, "%s: unable to open '%s': %s\n",
            APP_NAME, filename, strerror(errno));
        return EXIT_FAILURE;
    }

    /* Get file size in case it is less than bytes requested. */
    fseek(fd, 0, SEEK_END);
    const long int fs_bytes = ftell(fd);

    if (fs_bytes < n_bytes) {
        /* If it is less, just rewind the FP. */
        rewind(fd);
    } else {
        /* Otherwise, position read head to n_bytes from the end. */
        if (fseek(fd, fs_bytes - n_bytes, SEEK_SET) != 0) {
            fprintf(stderr, "%s: unable to seek to end of '%s': %s\n",
                APP_NAME, filename, strerror(errno));
            return EXIT_FAILURE;
        }
    }

    int ch;
    while ((ch = fgetc(fd))) {
        if (ch == EOF) {
        break;
        }
        printf("%c", ch);
    }

    fclose(fd);
    return EXIT_SUCCESS;
}

static int tail_lines(char *filename, const int32_t n_lines)
{
    if (opts.verbose) {
        printf("==> %s%s%s <==\n", ANSI_BLUE_B, filename, ANSI_RESET);
    }

    FILE *fd = fopen(filename, "r");
    if (fd == NULL) {
        fprintf(stderr, "Unable to open '%s': %s\n", filename, strerror(errno));
        return EXIT_FAILURE;
    }

    int lines = 0;
    int ch;
    while ((ch = getc(fd)) != EOF) {
        if (ch == '\n') {
            lines++;
        }
    }
    rewind(fd);
    char buffer[MAX_LINE_LENGTH];

    /* If total lines is less than requested lines - print entire file. */
    if (lines <= n_lines) {
        while (fgets(buffer, sizeof(buffer), fd) != NULL) {
            printf("%s", buffer);
        }
    } else {
      const long int first_line = lines - n_lines;
        int current_line = 0;
        while (fgets(buffer, sizeof(buffer), fd) != NULL) {
            current_line++;
            if (current_line > first_line) {
                printf("%s", buffer);
            }
        }
    }

    fclose(fd);
    return EXIT_SUCCESS;
}

int main(const int argc, char *argv[]) {
    const struct option long_opts[] = {
        { .name = "help",    .has_arg = no_argument,       .flag = nullptr, .val = 'h' },
        { .name = "version", .has_arg = no_argument,       .flag = nullptr, .val = 'V' },
        { .name = "lines",   .has_arg = required_argument, .flag = nullptr, .val = 'n' },
        { .name = "bytes",   .has_arg = required_argument, .flag = nullptr, .val = 'b' },
        { .name = "quiet",   .has_arg = no_argument,       .flag = nullptr, .val = 'q' },
        { .name = "verbose", .has_arg = no_argument,       .flag = nullptr, .val = 'v' },
        { .name = nullptr,   .has_arg = no_argument,       .flag = nullptr, .val = 0 }
    };

    /* Default lines to tail. */
    uint32_t n_units = 10;
    /* Min and max vals for parse_numeric_arg. */
    const int *min = &(int){1};
    const int *max = &(int){INT32_MAX};

    int opt;
    while ((opt = getopt_long(argc, argv, "Vhn:b:qv", long_opts, nullptr)) != -1) {
      switch (opt) {
      case 'V':
        printf("%s (%s) version %s\n", APP_NAME, APP_SUITE, APP_VERSION);
        printf("%s compiled on %s at %s\n",
               strrchr(__FILE__, '/') ? strrchr(__FILE__, '/') + 1 : __FILE__,
               __DATE__, __TIME__);
        return EXIT_SUCCESS;
      case 'h':
        show_help();
        return EXIT_SUCCESS;
      case 'v':
        opts.verbose = true;
        opts.quiet = false;
        break;
      case 'q':
        opts.quiet = true;
        opts.verbose = false;
        break;
      case 'n':
        opts.lines = true;
        opts.bytes = false;
        n_units = (uint32_t)parse_numeric_arg(optarg, min, max, "tail");
        break;
      case 'b':
        opts.lines = false;
        opts.bytes = true;
        n_units = (uint32_t)parse_numeric_arg(optarg, min, max, "tail");
        break;
      default:
        show_help();
        return EXIT_FAILURE;
      }
    }

    const int n_file_args = argc - optind;
    if (n_file_args < 1) {
        fprintf(stderr, "%s: no file specified\n", APP_NAME);
        show_help();
        return EXIT_FAILURE;
    }

    /* Toggle the header for multiple files if not --quiet. */
    if (n_file_args >= 2) {
        if (!opts.quiet) {
            opts.verbose = 1;
        }
    }

    for (; optind < argc; optind++) {
        if (opts.bytes) {
            tail_bytes(argv[optind], n_units);
        } else {
            tail_lines(argv[optind], n_units);
        }
    }
    return EXIT_SUCCESS;
}
