/***************************************************************************
 *   head.c - print first n lines or bytes of file                         *
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

#include "common.h"
#include <sys/fcntl.h>

#define MAX_LINE_LENGTH 2048

const char *APPNAME =  "head";

struct opt_struct {
    unsigned int quiet:1;
    unsigned int verbose:1;
    unsigned int bytes:1;
    unsigned int lines:1;
} opts;

static void show_help(void) {
    printf("Usage: %s [OPTION]... FILE [FILE...]\n\n\
Options:\n\
    -n, --lines=N\tprint first N lines\n\
    -b, --bytes=N\tprint first N bytes instead of lines\n\
    -v, --verbose\talways print file header(s)\n\
    -q, --quiet\t\tnever print file header(s)\n\
    -h, --help\t\tdisplay this help\n\
    -V, --version\tdisplay version information\n\n\
Report bugs to <bulliver@gmail.com>\n", APPNAME);
}

int head_bytes(char *filename, long int n_bytes) {
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

int head_lines(char *filename, long int n_lines) {
    if (opts.verbose) {
        printf("==> %s%s%s <==\n", ANSI_BLUE_B, filename, ANSI_RESET);
    }

    FILE *fd = fopen(filename, "r");
    if (fd == NULL) {
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
        {"help", 0, NULL, 'h'},
        {"version", 0, NULL, 'V'},
        {"lines", required_argument, NULL, 'n'},
        {"bytes", required_argument, NULL, 'b'},
        {"quiet", 0, NULL, 'q'},
        {"verbose", 0, NULL, 'v'},
        {0,0,0,0}
    };

    /* defaults */
    long int n_units = 10;
    opts.lines = 1;
    opts.bytes = 0;
    opts.quiet = 0;
    opts.verbose = 0;

    while ((opt = getopt_long(argc, argv, "Vhn:b:qv", long_opts, NULL)) != -1) {
      switch (opt) {
      case 'V':
        printf("%s (%s) version %s\n", APPNAME, APPSUITE, APPVERSION);
        printf("%s compiled on %s at %s\n",
               strrchr(__FILE__, '/') ? strrchr(__FILE__, '/') + 1 : __FILE__,
               __DATE__, __TIME__);
        exit(EXIT_SUCCESS);
      case 'h':
        show_help();
        exit(EXIT_SUCCESS);
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
        n_units = strtol(optarg, NULL, 10);
        break;
      case 'b':
        opts.lines = 0;
        opts.bytes = 1;
        n_units = strtol(optarg, NULL, 10);
        break;
      case ':':
      case '?':
        /* getopt_long prints own error message */
        exit(EXIT_FAILURE);
      default:
        show_help();
        exit(EXIT_FAILURE);
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
