/***************************************************************************
 *   cat.c - concatenate files and print on the standard output            *
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

#define APPNAME "cat"
#include "common.h"


int main (int argc, char *argv[]) {

    int n;
    if (argc == 1) {
        cat_file("/dev/stdin");
    }
    for ( n = 1; n < argc; n = n + 1 )
        cat_file(argv[n]);
        return 0;
}

int cat_file(char *filename) {

    FILE *name;
    char c;

    if ( (name = fopen(filename, "r")) == NULL ) {
        f_error(filename, NULL);
    }

    while (( c = getc(name)) != EOF )
        putc(c, stdout);

    fclose(name);
    return EXIT_SUCCESS;
}
