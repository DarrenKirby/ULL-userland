/***************************************************************************
 *   tail.c - print last n lines or bytes of file                          *
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

#include <sys/fcntl.h>
#include "common.h"

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
Print last N lines or bytes of file\n\n\
Options:\n\
    -n, --lines=N\t\t print first N lines\n\
    -b, --bytes=N\t\t print first N bytes instead of lines\n\
    -v, --verbose\t\t always print file header(s)\n\
    -q, --quiet\t\t never print file header(s)\n\
    -h, --help\t\t display this help\n\
    -V, --version\t display version information\n\n\
Report bugs to <bulliver@gmail.com>\n", APPNAME);
}

int tail_bytes(char *filename, long int n_bytes) {
    if (opts.verbose) {
        printf("==> %s%s%s <==\n", ANSI_BLUE_B, filename, ANSI_RESET);
    }

    FILE *fd = fopen(filename, "r");
    if (fd == NULL) {
        fprintf(stderr, "Unable to open '%s': %s\n", filename, strerror(errno));
        return EXIT_FAILURE;
    }

    /* get filesize in case it is less than bytes requested */
    fseek(fd, 0, SEEK_END);
    const long int fs_bytes = ftell(fd);

    if (fs_bytes < n_bytes) {
        /* if it is less, just rewind the FP */
        rewind(fd);
    } else {
        /* otherwise, position read head to n_bytes from the end */
        if (fseek(fd, -n_bytes, SEEK_END) != 0) {
            fprintf(stderr, "Unable to seek to end of '%s': %s\n", filename, strerror(errno));
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

    printf("\n");
    fclose(fd);
    return EXIT_SUCCESS;
}

int tail_lines(char *filename, long int n_lines) {
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

    /* If total lines is less than requested lines - print entire file */
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
    int opt;

    const struct option long_opts[] = {
        {"help", 0, NULL, 'h'},
        {"version", 0, NULL, 'V'},
        {"lines", required_argument, NULL, 'n'},
        {"bytes", required_argument, NULL, 'b'},
        {"quiet", 0, NULL, 'q'},
        {"verbose", 0, NULL, 'v'},
        {NULL,0,NULL,0}
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
            tail_bytes(argv[optind], n_units);
        } else {
            tail_lines(argv[optind], n_units);
        }
    }
    return EXIT_SUCCESS;
}
