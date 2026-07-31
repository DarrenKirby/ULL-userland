/***************************************************************************
 *   tee.c - duplicate standard input                                      *
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
#include <fcntl.h>

#include "common.h"

#define BUFFER_SIZE 1024


static const char *APP_NAME =  "tee";

static void show_help() {
    printf("Usage: %s [OPTION]...[FILE...]\n\n\
Options:\n\
    -a, --append\tappend to file arguments if they exist\n\
    -h, --help\t\tdisplay this help\n\
    -V, --version\tdisplay version information\n\n\
Report bugs to <darren@dragonbyte.ca>\n", APP_NAME);
}

int main(const int argc, char *argv[])
{
    const struct option longopts[] = {
        { .name = "help",    .has_arg = no_argument, .flag = nullptr, .val = 'h' },
        { .name = "version", .has_arg = no_argument, .flag = nullptr, .val = 'V' },
        { .name = "append",  .has_arg = no_argument, .flag = nullptr, .val = 'a' },
        { .name = nullptr,   .has_arg = no_argument, .flag = nullptr, .val = 0 }
    };

    int opt;
    int append = false;
    while ((opt = getopt_long(argc, argv, "Vha", longopts, nullptr)) != -1) {
        switch(opt) {
        case 'V':
            printf("%s (%s) version %s\n", APP_NAME, APP_SUITE, APP_VERSION);
            printf("%s compiled on %s at %s\n",
                   strrchr(__FILE__, '/') ? strrchr(__FILE__, '/') + 1 : __FILE__,
                   __DATE__, __TIME__);
            return EXIT_SUCCESS;
            break;
        case 'h':
            show_help();
            return EXIT_SUCCESS;
            break;
        case 'a':
            append = true;
            break;
        default:
            show_help();
            return EXIT_FAILURE;
            break;
        }
    }

    /* Number of outfile args. */
    int n_ofiles = argc - optind;

    /* Allocate an array to store file descriptors. */
    int *outfiles = malloc(n_ofiles * sizeof(int));
    if (!outfiles) {
        fprintf(stderr, "%s: failed to allocate memory!\n", APP_NAME);
        return EXIT_FAILURE;
    }

    /* Open each output file and store the file descriptor. */
    for (int i = 0; i < n_ofiles; i++) {
        if (append) {
            outfiles[i] = open(argv[optind + i], O_WRONLY | O_CREAT | O_APPEND, 0644);
        } else {
            outfiles[i] = open(argv[optind + i], O_WRONLY | O_CREAT | O_TRUNC, 0644);
        }

        if (outfiles[i] < 0) {
            fprintf(stderr, "%s: failed to open() %s: %s",
                APP_NAME, argv[optind + i], strerror(errno));
            free(outfiles);
            return EXIT_FAILURE;
        }
    }

    /* Read from stdin and write to stdout and all outfiles. */
    char buffer[BUFFER_SIZE];
    ssize_t bytesRead;
    while ((bytesRead = read(STDIN_FILENO, buffer, sizeof(buffer))) > 0) {
        ssize_t result = write(STDOUT_FILENO, buffer, bytesRead);
        if (result == -1) {
            perror("write");
            return EXIT_FAILURE;
        }
        for (int i = 0; i < n_ofiles; i++) {
            if (write(outfiles[i], buffer, bytesRead) != bytesRead) {
                fprintf(stderr, "%s: write() failed: %s\n", APP_NAME, strerror(errno));
                free(outfiles);
                return EXIT_FAILURE;
            }
        }
    }

    if (bytesRead < 0) {
        perror("Read from stdin failed");
    }

    /* Close and deallocate all the outfiles. */
    for (int i = 0; i < n_ofiles; i++) {
        close(outfiles[i]);
    }
    free(outfiles);

    return EXIT_SUCCESS;
}


