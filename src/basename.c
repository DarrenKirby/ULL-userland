/***************************************************************************
 *   basename.c - strip directory and suffix from filenames                *
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


#define APPNAME "basename"
#include "common.h"

static void show_help(void) {
    printf("Usage: %s PATH\n \
    or: %s [OPTION] [PATH]\n\n \
    -s, --suffix=SUFFIX\tremove trailing suffix\n \
    -h, --help\t\t\tdisplay this help\n \
    -V, --version\t\tdisplay version information\n\n \
    Examples:\n \
    \tbasename /usr/bin/sort         Output: 'sort'.\n \
    \tbasename -s .h include/stdio.h Output: 'stdio'. \n\n \
    Report bugs to <bulliver@gmail.com>\n", APPNAME, APPNAME);
}

int main(int argc, char *argv[]) {
    int  opt;
    char name[256];
    int  sfx = 0;
    int  sdn = 0;
    char suffix[256];

    struct option longopts[] = {
        {"suffix", required_argument, NULL, 's'},
        {"help", 0, NULL, 'h'},
        {"version", 0, NULL, 'V'},
        {0,0,0,0}
    };

    while ((opt = getopt_long(argc, argv, "Vs:h", longopts, NULL)) != -1) {
        switch(opt) {
            case 's':
                sfx = 1;
                strncpy(suffix, optarg, 256);
                break;
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
    int n_args = argc - optind; /* Number of arguments */

    if (n_args == 0 || strcmp(argv[optind], "-") == 0) {          /* Read path from STDIN */
        fgets(name, 256, stdin);
        sdn = 1;
    } else {
	    strcpy(name, argv[optind]);
    }

    /* Next 10 lines stolen from GNU basename */
    if (sfx) {
        char *np;
        const char *sp;
        np = name + strlen(name);
        sp = suffix + strlen(suffix);

        while (np > name && sp > suffix)
            if (*--np != *--sp)
                return 0;
        if (np > name)
            *np = '\0';
    }

    printf("%s%s", basename(name), sdn == 0 ? "\n" : "");
    return EXIT_SUCCESS;
}


