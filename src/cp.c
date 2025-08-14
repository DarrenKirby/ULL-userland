/***************************************************************************
 *   cp.c - copy files to a new location                                   *
 *                                                                         *
 *   Copyright (C) 2014-2025 by Darren Kirby                               *
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

#include <fcntl.h>
#include "common.h"

const char *APPNAME = "cp";

#define BUFF_SIZE 4096


static void show_help(void) {
    printf("Usage: %s [OPTION] file1 file2 [file1 dir1]\n\n \
    -h, --help\t\tdisplay this help\n \
    -V, --version\tdisplay version information\n\n \
    Report bugs to <bulliver@gmail.com>\n", APPNAME);
}

int main(int argc, char *argv[]) {
    int opt;

    struct option longopts[] = {
        {"help", 0, NULL, 'h'},
        {"version", 0, NULL, 'V'},
        {0,0,0,0}
    };

    while ((opt = getopt_long(argc, argv, "Vh", longopts, NULL)) != -1) {
        switch(opt) {
            case 'V':
                printf("%s (%s) version %s\n", APPNAME, APPSUITE, APPVERSION);
                printf("%s compiled on %s at %s\n", basename(__FILE__), __DATE__, __TIME__);
                exit(EXIT_SUCCESS);
                break;
            case 'h':
                show_help();
                exit(EXIT_SUCCESS);
                break;
            default:
                show_help();
                exit(EXIT_FAILURE);
                break;
        }
    }

    char buf[BUFF_SIZE];

    if (argc != 3) {
        printf("Usage: %s source destination\n\n", argv[0]);
        printf("\twhere `source` is a file and `destination` is a file  or directory\n");
        return -1;
    }

    int fd1, fd2;
    int n;
    ssize_t bytes_read = 0;
    // -rw-r--r--
    mode_t open_flags =  S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH;

    if ((fd1 = open(argv[1], O_RDONLY)) == -1) {
        fprintf(stderr, "Unable to open '%s': %s\n", argv[1], strerror(errno));
        return EXIT_FAILURE;
    }

    if ((fd2 = open(argv[2], O_WRONLY| O_CREAT, open_flags)) == -1) {
        fprintf(stderr, "Unable to open '%s': %s\n", argv[2], strerror(errno));
        return EXIT_FAILURE;
    }

    while ((n = read(fd1, &buf, BUFF_SIZE)) > 0) {
        if (n < 0) {
            fprintf(stderr, "Unable to read '%s': %s\n", argv[1], strerror(errno));
            return EXIT_FAILURE;
        }

        bytes_read += n;
        if (bytes_read % 1000 == 0)
            printf("Read %li bytes\n", bytes_read);

        if (n > 0) {
            if ((write(fd2, &buf, n)) < 0) {
                fprintf(stderr, "Unable to write '%s': %s\n", argv[2], strerror(errno));
                return EXIT_FAILURE;
            }
        }
    }

    close(fd1);
    close(fd2);


    return EXIT_SUCCESS;
}
