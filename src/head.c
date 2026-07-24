/***************************************************************************
 *   head.c - print first n lines or bytes of file                         *
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
#include <sys/fcntl.h>

#include "common.h"

#define MAX_LINE_LENGTH 2048

static const char *APP_NAME =  "head";

static struct {
    bool quiet:1;
    bool verbose:1;
    bool bytes:1;
    bool lines:1;
} opts = {
    .lines = true,
    .verbose = false,
    .bytes = false,
    .quiet = false };

static void show_help()
{
    printf("Usage: %s [OPTION]... FILE [FILE...]\n\n\
Options:\n\
    -n, --lines=N\tprint first N lines\n\
    -b, --bytes=N\tprint first N bytes instead of lines\n\
    -v, --verbose\talways print file header(s)\n\
    -q, --quiet\t\tnever print file header(s)\n\
    -h, --help\t\tdisplay this help\n\
    -V, --version\tdisplay version information\n\n\
Report bugs to <bulliver@gmail.com>\n", APP_NAME);
}

static int head_bytes(char *filename, const long int n_bytes)
{
    if (opts.verbose) {
        printf("==> %s%s%s <==\n", ANSI_BLUE_B, filename, ANSI_RESET);
    }

    FILE *fd = fopen(filename, "r");
    if (fd == NULL) {
        fprintf(stderr, "Unable to open '%s': %s\n", filename, strerror(errno));
        return EXIT_FAILURE;
    }

    for (long int i = 0; i < n_bytes; i++) {
        const int ch = fgetc(fd);
        if (ch == EOF) {
            return EXIT_SUCCESS;
        }
        printf("%c", ch);
    }
    printf("\n");
    fclose(fd);
    return EXIT_SUCCESS;
}

static int head_lines(char *filename, long int n_lines)
{
    if (opts.verbose) {
        printf("==> %s%s%s <==\n", ANSI_BLUE_B, filename, ANSI_RESET);
    }

    FILE *fd = fopen(filename, "r");
    if (!fd) {
        fprintf(stderr, "Unable to open '%s': %s\n", filename, strerror(errno));
        return EXIT_FAILURE;
    }

    char buffer[MAX_LINE_LENGTH];
    while (fgets(buffer, sizeof(buffer), fd) != NULL) {
        if (n_lines == 0) {
            return EXIT_SUCCESS;
        }
        printf("%s", buffer);
        n_lines--;
    }
    fclose(fd);
    return EXIT_SUCCESS;
}

int main(const int argc, char *argv[]) {
    int opt;

    const struct option long_opts[] = {
        {.name = "help",    .has_arg = no_argument,       .flag = nullptr, .val = 'h'},
        {.name = "version", .has_arg = no_argument,       .flag = nullptr, .val = 'V'},
        {.name = "lines",   .has_arg = required_argument, .flag = nullptr, .val = 'n'},
        {.name = "bytes",   .has_arg = required_argument, .flag = nullptr, .val = 'b'},
        {.name = "quiet",   .has_arg = no_argument,       .flag = nullptr, .val = 'q'},
        {.name = "verbose", .has_arg = no_argument,       .flag = nullptr, .val = 'v'},
        {.name = nullptr,   .has_arg = no_argument,       .flag = nullptr, .val = 0}
    };

    /* Print 10 lines by default. */
    long int n_units = 10;

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
        opts.verbose = 1;
        opts.quiet = 0;
        break;
      case 'q':
        opts.quiet = 1;
        opts.verbose = 0;
        break;
      case 'n':
        opts.lines = 1;
        opts.bytes = 0;
        n_units = strtol(optarg, nullptr, 10);
        break;
      case 'b':
        opts.lines = 0;
        opts.bytes = 1;
        n_units = strtol(optarg, nullptr, 10);
        break;
      default:
        show_help();
        return EXIT_FAILURE;
      }
    }

    const int n_file_args = argc - optind;
    if (n_file_args < 1) {
        printf("No file specified\n");
        show_help();
        return EXIT_FAILURE;
    }

    /* toggle the header for multiple files if not --quiet */
    if (n_file_args >= 2) {
        if (!opts.quiet) {
            opts.verbose = 1;
        }
    }

    for (; optind < argc; optind++) {
        if (opts.bytes) {
            head_bytes(argv[optind], n_units);
        } else {
            head_lines(argv[optind], n_units);
        }
    }
    return EXIT_SUCCESS;
}
