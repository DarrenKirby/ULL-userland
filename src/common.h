/***************************************************************************
 *   common.h - includes and functions common to all files                 *
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

#ifndef _COMMON_H
#define _COMMON_H

/* Common includes */
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <limits.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>

#ifdef PATH_MAX
static int pathmax = PATH_MAX;
#else
static int pathmax = 0;
#endif

/* For longopts */
#define _GNU_SOURCE
#include <getopt.h>

/* Version information */
#define APPSUITE   "dgnu-utils"
#define APPVERSION "0.2"

/* prototypes */

void g_error(char *message);
int f_error(char *filename, char *message);
int dump_args(int argc, char *argv[]);
char *trim_whitespace(char *str);

/* Generic error */
void g_error(char *message) {
    printf("%s: %s\n", APPNAME, message);
}

/* File error */
int f_error(char *filename, char *message) {
    char error[50];
    sprintf(error, "%s: %s%s", APPNAME, (message == NULL) ? "" : message, filename);
    perror(error);
    exit(EXIT_FAILURE);
}

/* Debugging aids */
int dump_args(int argc, char *argv[]) {
    int i;
    printf("argc: %i\n", argc);
    for (i = 1; i < argc; i++) {
        printf("argv[%i]: %s\n", i, argv[i]);
    }
    return 0;
}

/* allocate memory for pathnames. This is from APUE */
#define SUSV3          200112L
#define PATH_MAX_GUESS 1024
static long posix_version = 0;

char *path_alloc(int *sizep) {
    char *ptr;
    int size;
    
    if (posix_version == 0)
        posix_version = sysconf(_SC_VERSION);
    
    if (pathmax == 0) {
        errno = 0;
        if ((pathmax = pathconf("/", _PC_PATH_MAX)) < 0) {
            if (errno == 0)
                pathmax = PATH_MAX_GUESS;
            else
                g_error("pathconf error for _PC_PATH_MAX");
        } else {
            pathmax++;
        }
    }
    if (posix_version < SUSV3)
        size = pathmax + 1;
    else
        size = PATH_MAX;
    
    if ((ptr = malloc(size)) == NULL)
        g_error("malloc error for pathname");
        
    if (sizep != NULL)
        *sizep = size;
    return(ptr);
}
/* end APUE code */
    
/* trims leading and tailing whitespace from strings */
char *trim_whitespace(char *str) {
    size_t len = 0;
    char *frontp = str;
    char *endp = NULL;

    if( str == NULL ) { return NULL; }
    if( str[0] == '\0' ) { return str; }

    len = strlen(str);
    endp = str + len;

    /* Move the front and back pointers to address the first non-whitespace
     * characters from each end.
     */
    while( isspace(*frontp) ) { ++frontp; }
    if( endp != frontp )
    {
        while( isspace(*(--endp)) && endp != frontp ) {}
    }

    if( str + len - 1 != endp )
            *(endp + 1) = '\0';
    else if( frontp != str &&  endp == frontp )
            *str = '\0';

    /* Shift the string so that it starts at str so that if it's dynamically
     * allocated, we can still free it on the returned pointer.  Note the reuse
     * of endp to mean the front of the string buffer now.
     */
    endp = str;
    if( frontp != str )
    {
            while( *frontp ) { *endp++ = *frontp++; }
            *endp = '\0';
    }
    
    return str;
}


#endif /* _COMMON_H */
