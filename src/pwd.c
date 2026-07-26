/***************************************************************************
 *   pwd.c - print name of current/working directory                       *
 *                                                                         *
 *   Copyright (C) 2014-2026 by Darren Kirby                               *
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
/* TODO: more cleanup. */
#include <getopt.h>

#include "common.h"


static const char *APP_NAME = "pwd";

static void show_help()
{
    printf("Usage: %s [OPTION]\n\n\
    Print the full filename of the current working directory.\n\n\
Options:\n\
    -h, --help\t\tdisplay this help\n\
    -V, --version\tdisplay version information\n\n\
Report bugs to <bulliver@gmail.com>\n", APP_NAME);
}

int main(int argc, char *argv[])
{
    const struct option longopts[] = {
        {"help", 0, NULL, 'h'},
        {"version", 0, NULL, 'V'},
        {0,0,0,0}
    };

    int opt;
    while ((opt = getopt_long(argc, argv, "Vh", longopts, NULL)) != -1) {
        switch(opt) {
            case 'V':
                printf("%s (%s) version %s\n", APP_NAME, APP_SUITE, APP_VERSION);
                printf("%s compiled on %s at %s\n",
                       strrchr(__FILE__, '/') ? strrchr(__FILE__, '/') + 1 : __FILE__,
                       __DATE__, __TIME__);
                exit(EXIT_SUCCESS);
            case 'h':
                show_help();
                exit(EXIT_SUCCESS);
            default:
                show_help();
                exit(EXIT_FAILURE);
        }
    }

    const size_t path_max = get_path_max();
    char cwd[path_max];
    char *cwd_p = cwd;

    if (getcwd(cwd_p, path_max) == NULL)
        fprintf(stderr, "%s: getcwd() failed: %s\n", APP_NAME, strerror(errno));

    printf("%s\n", cwd);

    return EXIT_SUCCESS;
}
