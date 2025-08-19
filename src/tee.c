/***************************************************************************
 *   tee.c - duplicate standard input                                      *
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
#include <fcntl.h>

#define BUFFER_SIZE 1024
const char *APPNAME =  "tee";


static void show_help(void) {
    printf("Usage: %s [OPTION]...[FILE...]\n\n\
Options:\n\
    -a, --append\tappend to file arguments if they exist\n\
    -h, --help\t\tdisplay this help\n\
    -V, --version\tdisplay version information\n\n\
Report bugs to <bulliver@gmail.com>\n", APPNAME);
}

int main(int argc, char *argv[]) {
    int opt;
    int append = 0;

    struct option longopts[] = {
        {"help", 0, NULL, 'h'},
        {"version", 0, NULL, 'V'},
        {"append", 0, NULL, 'a'},
        {0,0,0,0}
    };

    while ((opt = getopt_long(argc, argv, "Vha", longopts, NULL)) != -1) {
        switch(opt) {
            case 'V':
                printf("%s (%s) version %s\n", APPNAME, APPSUITE, APPVERSION);
                printf("%s compiled on %s at %s\n",
                       strrchr(__FILE__, '/') ? strrchr(__FILE__, '/') + 1 : __FILE__,
                       __DATE__, __TIME__);
                exit(EXIT_SUCCESS);
                break;
            case 'h':
                show_help();
                exit(EXIT_SUCCESS);
                break;
            case 'a':
                append = 1;
                break;
            case ':':
                 /* getopt_long prints own error message */
                exit(EXIT_FAILURE);
                break;
            case '?':
                 /* getopt_long prints own error message */
                exit(EXIT_FAILURE);
            default:
                show_help();
                exit(EXIT_FAILURE);
                break;
        }
    }

    // number of outfile args
    int n_ofiles = argc - optind;

    // Dynamically allocate an array to store file descriptors
    int *outfiles = malloc(n_ofiles * sizeof(int));
    if (outfiles == NULL) {
        perror("Failed to allocate memory");
        return 1;
    }

    // Open each output file and store the file descriptor
    for (int i = 0; i < n_ofiles; i++) {
        if (append == 1) {
            outfiles[i] = open(argv[optind + i], O_WRONLY | O_CREAT | O_APPEND, 0644);
        } else {
            outfiles[i] = open(argv[optind + i], O_WRONLY | O_CREAT | O_TRUNC, 0644);
        }
        if (outfiles[i] < 0) {
            perror("Failed to open output file");
            free(outfiles); // Clean up if any file fails to open
            return 1;
        }
    }

    // Read from stdin and write to stdout and all outfiles
    char buffer[BUFFER_SIZE];
    ssize_t bytesRead;
    while ((bytesRead = read(STDIN_FILENO, buffer, sizeof(buffer))) > 0) {
        write(STDOUT_FILENO, buffer, bytesRead);
        for (int i = 0; i < n_ofiles; i++) {
            if (write(outfiles[i], buffer, bytesRead) != bytesRead) {
                perror("Write failed");
                free(outfiles); // Clean up on error
                return 1;
            }
        }
    }

    if (bytesRead < 0) {
        perror("Read from stdin failed");
    }

    // Close all the outfiles
    for (int i = 0; i < n_ofiles; i++) {
        close(outfiles[i]);
    }

    free(outfiles); // Free the allocated memory for file descriptors
    return 0;
}


