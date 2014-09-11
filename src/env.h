/***************************************************************************
 *   env.h - includes and functions common to env.c and printenv.c         *
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

#ifndef _ENV_H
#define _ENV_H

/* Cycle through and print all env variables */
int print_all_env(void) {
    extern char **environ;
    char **env = environ;
    while (*env) {
        printf("%s\n", *env);
        env++;
    }
    return EXIT_SUCCESS;
}
/* Print a single env variable */
int print_env(char *name) {
    char *var, *value;
    var = name;
    value = getenv(var);
    if (value)
        printf("%s\n", value);
    else
        return EXIT_FAILURE;
    return EXIT_SUCCESS;
}

#endif /* _ENV_H */
