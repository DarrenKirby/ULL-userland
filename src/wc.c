/***************************************************************************
 *   wc.c - print newline, word, and byte counts                           *
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

#include "common.h"


static const char *APP_NAME = "wc";

/* Our count struct is used both for storing boolean
   values regarding _if_ we want to count something,
   and it also holds the counts themselves */
struct count {
    unsigned int chars;
    unsigned int words;
    unsigned int lines;
    unsigned int longest;
};

static void showHelp()
{
    printf("Usage: %s [OPTION]... [FILE]...\n\n\
    Print newline, word, and byte counts for each FILE, and a total line if more than\n\
    one FILE is specified.  With no FILE, read standard input.\n\n\
Options:\n\
    -c, --bytes\t\t  print the byte counts\n\
    -m, --chars\t\t  print the character counts\n\
    -l, --lines\t\t  print the newline counts \n\
    -w, --words\t\t  print the word counts \n\
    -L, --max-line-length print the length of the longest line\n\
    -h, --help\t\t  display this help and exit\n\
    -V, --version\t  output version information and exit\n\n\
Report bugs to <darren@dragonbyte.ca>\n", APP_NAME);
}

static struct count count_all(char *filename)
{
    FILE *fp;

    if ((fp = fopen(filename, "r")) == NULL ) {
        fprintf(stderr, "%s: error opening %s: %s\n",
            APP_NAME, filename, strerror(errno));
        exit(EXIT_FAILURE);
    }

    struct count t_counts = { .chars = 0, .words = 0, .lines = 0, .longest = 0 };

    int c;
    int state = 0;
    unsigned int current_line_count = 0;


    while ((c = getc(fp)) != EOF) {
        current_line_count++;  /* Longest line char counter. */
        t_counts.chars++;      /* Everything is a char. */

        if (c == '\n') {
            ++t_counts.lines;
            /* We do not count the newline, so we remove
             * it from the comparison and assignment. */
            if (current_line_count - 1 > t_counts.longest) {
                t_counts.longest = current_line_count - 1;
            }
            current_line_count = 0;
        }

        /* A word boundary. */
        if (c == ' ' || c == '\n' || c == '\t')
            state = 0;
        else if (state == 0) {
            state = 1;
            ++t_counts.words;
        }

    }
    return t_counts;
}

int main(const int argc, char *argv[])
{
   const struct option longopts[] = {
        { .name = "help",            .has_arg = no_argument, .flag = nullptr, .val = 'h' },
        { .name = "bytes",           .has_arg = no_argument, .flag = nullptr, .val = 'c' },
        { .name = "chars",           .has_arg = no_argument, .flag = nullptr, .val = 'm' },
        { .name = "lines",           .has_arg = no_argument, .flag = nullptr, .val = 'l' },
        { .name = "words",           .has_arg = no_argument, .flag = nullptr, .val = 'w' },
        { .name = "version",         .has_arg = no_argument, .flag = nullptr, .val = 'V' },
        { .name = "max-line-length", .has_arg = no_argument, .flag = nullptr, .val = 'L' },
        { .name = nullptr,           .has_arg = no_argument, .flag = nullptr, .val = 0 }
    };

    int opt;
    struct count opts = { .chars = 0, .words = 0, .lines = 0, .longest = 0 };
    while ((opt = getopt_long(argc, argv, "hcmlLwV", longopts, NULL)) != -1) {
        switch(opt) {
            case 'L':
                opts.longest = 1;
                break;
            case 'c':  /* Bytes same as chars until Unicode support is added. */
            case 'm':
                opts.chars = 1;
                break;
            case 'l':
                opts.lines = 1;
                break;
            case 'w':
                opts.words = 1;
                break;
            case 'h':
                showHelp();
                return EXIT_SUCCESS;
            case 'V':
                printf("%s (%s) version %s\n", APP_NAME, APP_SUITE, APP_VERSION);
                printf("%s compiled on %s at %s\n",
                       strrchr(__FILE__, '/') ? strrchr(__FILE__, '/') + 1 : __FILE__,
                       __DATE__, __TIME__);
                return EXIT_SUCCESS;
            default:
                showHelp();
                return EXIT_FAILURE;
        }
    }

    /* If there are no options we count all but the longest line */
    if (opts.chars == 0 && opts.lines == 0 && opts.words == 0 && opts.longest == 0)
        opts.chars = opts.lines = opts.words = 1;

    struct count t_counts;
    struct count t_cumulative = { .chars = 0, .words = 0, .lines = 0, .longest = 0 };;
    bool multiple_args = false;

    if (argc == optind) {
        /* We're dealing with STDIN. Using /de/fd/0
         * is not POSIX, but APUE says this works on
         * *BSD, OS X. Linux and Solaris. */
        t_counts = count_all("/dev/fd/0");
        if (opts.lines)
            printf("%i ", t_counts.lines);
        if (opts.words)
            printf("%i ", t_counts.words);
        if (opts.chars)
            printf("%i ", t_counts.chars);
        if (opts.longest)
            printf("%i ", t_counts.longest);
        printf("\n");

    } else {
        /* Cycle through file arguments. */
        if (argc > optind + 1) {
            multiple_args = true;
        }

        while (optind < argc) {
            t_counts = count_all(argv[optind]);
            if (opts.lines)
                printf("%5i ", t_counts.lines);
            if (opts.words)
                printf("%5i ", t_counts.words);
            if (opts.chars)
                printf("%5i ", t_counts.chars);
            if (opts.longest)
                printf("%5i ", t_counts.longest);
            printf("%s\n", argv[optind]);
            optind++;

            t_cumulative.lines   += t_counts.lines;
            t_cumulative.words   += t_counts.words;
            t_cumulative.chars   += t_counts.chars;

            if (t_counts.longest > t_cumulative.longest)
                t_cumulative.longest = t_counts.longest;
        }
    }

    /* If there were multiple files, print the totals line. */
    if (multiple_args) {
        if (opts.lines)
            printf("%5i ", t_cumulative.lines);
        if (opts.words)
            printf("%5i ", t_cumulative.words);
        if (opts.chars)
            printf("%5i ", t_cumulative.chars);
        if (opts.longest)
            printf("%5i ", t_cumulative.longest);
        printf("total\n");
    }
    return EXIT_SUCCESS;
}
