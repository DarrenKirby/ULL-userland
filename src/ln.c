/***************************************************************************
 *   ln.c - make links between files                                       *
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

#define APPNAME "ln"
#include "common.h"

void show_help(void) {
    printf("Usage: %s [OPTION]... TARGET LINK_NAME\t(1st form)\n \
  or: %s [OPTION]... TARGET\t\t(2nd form)\n\n \
    -s, --symbolic\tmake symbolic links instead of hard links\n \
    -f, --force\tdo not prompt before overwriting files\n \
    -i, --interactive\tprompt before overwriting files\n \
    -v, --verbose\tprint out links created\n \
    -h, --help\t\tdisplay this help\n \
    -V, --version\tdisplay version information\n\n \
    Report bugs to <bulliver@gmail.com>\n", APPNAME, APPNAME);
}

int main(int argc, char *argv[]) {
    int opt;
    int symbolic = 0;
    int force = 0;
    int verbose = 0;
    int interactive = 0;

    struct option longopts[] = {
        {"symbolic", 0, NULL, 's'},
        {"force", 0, NULL, 'f'},
        {"interactive", 0, NULL, 'i'},
        {"verbose", 0, NULL, 'v'},
        {"help", 0, NULL, 'h'},
        {"version", 0, NULL, 'V'},
        {0,0,0,0}
    };

    while ((opt = getopt_long(argc, argv, "sfivVh", longopts, NULL)) != -1) {
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
            case 's':
                symbolic = 1;
                break;
            case 'f':
                force = 1;
                break;
            case 'v':
                verbose = 1;
                break;
            case 'i':
                interactive = 1;
                break;
            case ':':
                 /* getopt_long prints own error message */
                exit(EXIT_FAILURE);
                break;
            case '?':
                 /* getopt_long prints own error message */
                exit(EXIT_FAILURE);
            default : 
                show_help(); 
                exit(EXIT_FAILURE); 
                break;
        }
    }
    printf("Argc: %i\n", argc);
    /*if (argc > 3) {
        perror("too many arguments");
        exit(EXIT_FAILURE);
    }
    */
    char target[PATH_MAX];
    char *target_p = target; 
    strncpy(target_p, argv[argc], PATH_MAX);
    char name[PATH_MAX];
    char *name_p = name;
    strncpy(name_p, argv[argc + 1], PATH_MAX);

    if (argc) { /* 1st form */
        if (access(argv[1], F_OK) == 0) {
            if (force == 1) 
                unlink(argv[1]);
            if (interactive == 1) {
                char response; 
                printf("%s exits, overwrite? ('y' or 'n') ", argv[1]);
                do {
                response = getchar();
                } while (response == '\n');

                if (response == 'y' || response == 'Y')
                    unlink(argv[1]);
            }
        }
        if (symbolic == 1)
            symlink(argv[1], argv[2]);
        else
            link(argv[1], argv[2]);
        if (verbose == 1)
            printf("linked %s to %s\n", argv[1], argv[2]);
            

    } else if (argc == 2) { /* 2nd form */
        ;    
    } else {
        perror("not enough arguments");
        exit(EXIT_FAILURE);
    }
        
    return EXIT_SUCCESS;
}
