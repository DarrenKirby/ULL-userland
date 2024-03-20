/***************************************************************************
 *   cd.c - change the working directory                                   *
 *                                                                         *
 *   Copyright (C) 2014 - 2024 by Darren Kirby                             *
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

#define APPNAME "cd"

void show_help(void) {
    printf("Usage: %s [OPTION] [DIRECTORY]\n\n\
    Change current working directory to [DIRECTORY].\n\
    With no args, move to $HOME.\n\
    With '-' arg, move to $OLDPWD\n\n\
Options:\n\
    -h, --help\t\tdisplay this help\n\
    -V, --version\tdisplay version information\n\n\
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

    char pathbuf[PATHMAX + 1]; // account for null byte
    char *path = pathbuf;

    if (argc == 1) {
        path = getenv("HOME");
    } else if (strcmp(argv[1], "-") == 0) {
        path = getenv("OLDPWD");
    } else {
        strncpy(path, argv[1], PATHMAX);
    }

    /* due to the fact the shell runs all commands in a new process, we can't actually
     * change directories (chdir() changes the directory of the current process), thus,
     * the 'cd' command is a shell built-in. This program simply prints the directory
     * you would cd to if it were possible */
    printf("%s\n", path);

    if (chdir(path) != 0) {
        f_error(path, "Cannot cd to ");
    }
    return EXIT_SUCCESS;
}
