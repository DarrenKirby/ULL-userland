/***************************************************************************
 *   wc.c - print newline, word, and byte counts                           *
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

#define APPNAME "wc"
#include "common.h"

/* Our count struct is used both for storing boolean
   values regarding _if_ we want to count something,
   and it also holds the counts themselves */
struct count {
    unsigned int chars;
    unsigned int words;
    unsigned int lines;
    unsigned int longest;
};

void showHelp(void) {
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
Report bugs to <bulliver@gmail.com>\n", APPNAME);
}

/* The only way to get counts for STDIN is to
   count them all at once */
struct count count_all(char *filename) {
    FILE *fp;

    if ((fp = fopen(filename, "r")) == NULL ) {
        f_error(filename, "Cannot open ");
    }

    /* struct to return with all counts */
    struct count t_counts = {0,0,0,0};

    int c;
    int state = 0;
    int current_line_count = 0;


    while ((c = getc(fp)) != EOF) {
        current_line_count++;                   /* Longest line counter */
        t_counts.chars++;                       /* Everything is a char */

        if (c == '\n') {                        /* A newline */
            ++t_counts.lines;
            if (current_line_count > t_counts.longest) {
                t_counts.longest = current_line_count - 1; /* subtract the '\n' itself */
            }
            current_line_count = 0;             /* Reset our counter */
        }

        if (c == ' ' || c == '\n' || c == '\t') /* A word boundary */
            state = 0;
        else if (state == 0) {
            state = 1;
            ++t_counts.words;
        }

    }
    return t_counts;
}

int main(int argc, char *argv[]) {
    int opt;

    struct count count_opts = {0,0,0,0};

    struct option longopts[] = {
        {"help", 0, NULL, 'h'},
        {"bytes", 0, NULL, 'c'},
        {"chars", 0, NULL, 'm'},
        {"lines", 0, NULL, 'l'},
        {"words", 0, NULL, 'w'},
        {"version", 0, NULL, 'V'},
        {"max-line-length", 0, NULL, 'L'},
        {0,0,0,0}
    };

    while ((opt = getopt_long(argc, argv, "hcmlLwV", longopts, NULL)) != -1) {
        switch(opt) {
            case 'L':
                count_opts.longest = 1;
                break;
            case 'c':  /* Bytes same as chars */
            case 'm':
                count_opts.chars = 1;
                break;
            case 'l':
                count_opts.lines = 1;
                break;
            case 'w':
                count_opts.words = 1;
                break;
            case 'h':
                showHelp();
                exit(EXIT_SUCCESS);
            case 'V':
                printf("%s (%s) version %s\n", APPNAME, APPSUITE, APPVERSION);
                printf("%s compiled on %s at %s\n", basename(__FILE__), __DATE__, __TIME__);
                exit(EXIT_SUCCESS);
            default:
                showHelp();
                exit(EXIT_FAILURE);
        }
    }

    /* If there are no options we count all but longest line */
    if (count_opts.chars == 0 && count_opts.lines == 0 && count_opts.words == 0 && count_opts.longest == 0)
        count_opts.chars = count_opts.lines = count_opts.words = 1;

    struct count t_counts;
    struct count t_cumulative;
    int multiple_args = 0;
    if (argc == optind) {                     /* We're dealing with STDIN */
        t_counts = count_all("/dev/fd/0");    /* Not POSIX, but APUE says this works on *BSD, OS X. Linux and Solaris*/
        if (count_opts.lines == 1)
            printf("%i ", t_counts.lines);
        if (count_opts.words == 1)
            printf("%i ", t_counts.words);
        if (count_opts.chars == 1)
            printf("%i ", t_counts.chars);
        if (count_opts.longest == 1)
            printf("%i ", t_counts.longest);
        printf("\n");

    } else {                                  /* Cycle through file arguments */
        if (argc > (optind + 1)) {            /* more than one file arg  */
            t_cumulative = (struct count) {0,0,0,0};
            multiple_args = 1;
        }
        while (optind < argc) {
            t_counts = count_all(argv[optind]);
            if (count_opts.lines == 1)
                printf("%5i ", t_counts.lines);
            if (count_opts.words == 1)
                printf("%5i ", t_counts.words);
            if (count_opts.chars == 1)
                printf("%5i ", t_counts.chars);
            if (count_opts.longest == 1)
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
    if (multiple_args) {
        if (count_opts.lines == 1)
            printf("%5i ", t_cumulative.lines);
        if (count_opts.words == 1)
            printf("%5i ", t_cumulative.words);
        if (count_opts.chars == 1)
            printf("%5i ", t_cumulative.chars);
        if (count_opts.longest == 1)
            printf("%5i ", t_cumulative.longest);
        printf("total\n");
    }
    return EXIT_SUCCESS;
}
