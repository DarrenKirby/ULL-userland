/***************************************************************************
 *   pwd.c - print name of current/working directory                       *
 *                                                                         *
 *   Copyright (C) 2014-2015 by Darren Kirby                               *
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


#define APPNAME "pwd"
#include "common.h"

void show_help(void) {
    printf("Usage: %s [OPTION]\n\n\
    Print the full filename of the current working directory.\n\n\
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
            case 'h':
                show_help();
                exit(EXIT_SUCCESS);
            default:
                show_help();
                exit(EXIT_FAILURE);
        }
    }

    char cwd[PATHMAX];
    char *cwd_p;
    cwd_p = cwd;

    if (getcwd(cwd_p, PATHMAX) == NULL)
        g_error("getcwd failed");

    printf("%s\n", cwd);
    return EXIT_SUCCESS;
}
