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

/* Version information */
#define APPSUITE   "ull-userland"
#define APPVERSION "0.3"

/* Common includes */
#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <limits.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include <sys/stat.h>
#include <time.h>
#include <pwd.h>
#include <grp.h>
#include <libgen.h>

/* determine portable max path length */
#ifdef PATH_MAX
static int pathmax = PATH_MAX;
#else
static int pathmax = 0;
#endif

/* determine portable max filename length */
#ifdef _POSIX_NAME_MAX
static int filemax = _POSIX_NAME_MAX;
#else
static int filemax = 255;
#endif

/* For longopts */
#define _GNU_SOURCE
#include <getopt.h>

/* prototypes */
void g_error(char *message);                      /* Displays a general error */
void f_error(char *filename, char *message);      /* Displays a general error involving a file */
int  dump_args(int argc, char *argv[]);           /* Aid for debugging */
char *trim_whitespace(char *str);                 /* Removes leading and trailing whitespace from a string */
char *path_alloc(int *sizep);                     /* Allocates memory for a pathname */
char *file_perm_str(mode_t perm, int flags);      /* Displays a symbolic string of permission ie: rwxrw-rw- */
char *filetype(mode_t st_mode);                   /* Retuns the plain-english filetype from the stat struct */
int file_perm_oct(mode_t perm);                   /* Returns octal representation of permissions */
char *get_username(uid_t uid);                    /* Returns username from uid */
char *get_groupname(gid_t gid);                   /* Returns groupname from gid */

/* Comonly used function definitions */

/* Generic error */
void g_error(char *message) {
    printf("%s: %s\n", APPNAME, message);
}

/* File error */
void f_error(char *filename, char *message) {
    char error[50];
    sprintf(error, "%s: %s%s", APPNAME, (message == NULL) ? "" : message, filename);
    perror(error);
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
    if( endp != frontp ) {
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
    if( frontp != str ) {
            while( *frontp ) { *endp++ = *frontp++; }
            *endp = '\0';
    }
    return str;
}

#define FP_SPECIAL 1
/* Include set-user-ID, set-group-ID, and sticky
bit information in returned string */

#define STR_SIZE sizeof("rwxrwxrwx")

/* Return ls(1)-style string for file permissions mask, This is from
 * 'The Linux Programming Interface'
 */
char *file_perm_str(mode_t perm, int flags) {
    static char str[STR_SIZE];
    snprintf(str, STR_SIZE, "%c%c%c%c%c%c%c%c%c",
(perm & S_IRUSR) ? 'r' : '-', (perm & S_IWUSR) ? 'w' : '-',
(perm & S_IXUSR) ?
(((perm & S_ISUID) && (flags & FP_SPECIAL)) ? 's' : 'x') :
(((perm & S_ISUID) && (flags & FP_SPECIAL)) ? 'S' : '-'),
(perm & S_IRGRP) ? 'r' : '-', (perm & S_IWGRP) ? 'w' : '-',
(perm & S_IXGRP) ?
(((perm & S_ISGID) && (flags & FP_SPECIAL)) ? 's' : 'x') :
(((perm & S_ISGID) && (flags & FP_SPECIAL)) ? 'S' : '-'),
(perm & S_IROTH) ? 'r' : '-', (perm & S_IWOTH) ? 'w' : '-',
(perm & S_IXOTH) ?
(((perm & S_ISVTX) && (flags & FP_SPECIAL)) ? 't' : 'x') :
(((perm & S_ISVTX) && (flags & FP_SPECIAL)) ? 'T' : '-'));
    return str;
}

/* Returns octal permissions of a file/directory */
int file_perm_oct(mode_t perm) {
    int oct_perm = 00;
    (perm & S_ISUID) ? (oct_perm += 04000) : (oct_perm += 00);
    (perm & S_ISGID) ? (oct_perm += 02000) : (oct_perm += 00);
    (perm & S_ISVTX) ? (oct_perm += 01000) : (oct_perm += 00);
    (perm & S_IRUSR) ? (oct_perm +=  0400) : (oct_perm += 00);
    (perm & S_IWUSR) ? (oct_perm +=  0200) : (oct_perm += 00);
    (perm & S_IXUSR) ? (oct_perm +=  0100) : (oct_perm += 00);
    (perm & S_IRGRP) ? (oct_perm +=   040) : (oct_perm += 00);
    (perm & S_IWGRP) ? (oct_perm +=   020) : (oct_perm += 00);
    (perm & S_IXGRP) ? (oct_perm +=   010) : (oct_perm += 00);
    (perm & S_IROTH) ? (oct_perm +=    04) : (oct_perm += 00);
    (perm & S_IWOTH) ? (oct_perm +=    02) : (oct_perm += 00);
    (perm & S_IXOTH) ? (oct_perm +=    01) : (oct_perm += 00);
    return oct_perm;
}

char *filetype(mode_t st_mode) {
      switch (st_mode & S_IFMT) {
           case S_IFBLK:  return "block device";     break;
           case S_IFCHR:  return "character device"; break;
           case S_IFDIR:  return "directory";        break;
           case S_IFIFO:  return "FIFO/pipe";        break;
           case S_IFLNK:  return "symlink";          break;
           case S_IFREG:  return "regular file";     break;
           case S_IFSOCK: return "socket";           break;
           default:       return "unknown";          break;
           }
}

char *get_username(uid_t uid) {
    struct passwd *pwd;
    errno = 0;
    pwd = getpwuid(uid);
    if (pwd == NULL) {
        if (errno == 0) {
            return "unknown username";
        } else {
            g_error("username lookup failed");
            exit(EXIT_FAILURE);
        }
    }

    return pwd->pw_name;
}

char *get_groupname(gid_t gid) {
    struct group *grp;
    errno = 0;
    grp = getgrgid(gid);
    if (grp == NULL) {
        if (errno == 0) {
            return "unknown groupname";
        } else {
            g_error("groupname lookup failed");
            exit(EXIT_FAILURE);
        }
    }
    return grp->gr_name;
}

#endif /* _COMMON_H */
