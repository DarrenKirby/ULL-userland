/***************************************************************************
 *   common.h - includes and functions common to all files                 *
 *                                                                         *
 *   Copyright (C) 2014-2024 by Darren Kirby                               *
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

#ifndef _MY_COMMON_H
#define _MY_COMMON_H

/* Common includes */

#include <stdio.h>
//#include <utmp.h>
#include <sys/types.h>
#include <stdlib.h>
#include <errno.h>
#include <limits.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include <sys/stat.h>
//#include <time.h>
//#include <sys/time.h>
#include <pwd.h>
#include <grp.h>
#include <libgen.h>
//#include <fcntl.h>

/* Version information */
#define APPSUITE   "ull-userland"
#define APPVERSION "0.3"
/* External declaration of APPNAME
 * This solves all issues of trying to use a macro */
extern const char *APPNAME;

/* For OS X */
#if defined(__APPLE__) && defined(__MACH__)
#define st_atim      st_atimespec
#define st_mtim      st_mtimespec
#define st_ctim      st_ctimespec
#define st_birthtim  st_birthtimespec
#endif


/* determine portable max path length */
#ifdef _XOPEN_PATH_MAX
#define   PATHMAX   _XOPEN_PATH_MAX
#else
#define   PATHMAX   255
#endif

/* determine portable max filename length */
#ifdef _XOPEN_NAME_MAX
#define   FILEMAX  _XOPEN_NAME_MAX
#else
#define   FILEMAX  255
#endif

/*
 * ANSI colour codes
 */

/* normal */
#define ANSI_COLOR_RED     "\x1b[31m"
#define ANSI_COLOR_GREEN   "\x1b[32m"
#define ANSI_COLOR_YELLOW  "\x1b[33m"
#define ANSI_COLOR_BLUE    "\x1b[34m"
#define ANSI_COLOR_MAGENTA "\x1b[35m"
#define ANSI_COLOR_CYAN    "\x1b[36m"

/* bold */
#define ANSI_COLOR_RED_B     "\x1b[31;1m"
#define ANSI_COLOR_GREEN_B   "\x1b[32;1m"
#define ANSI_COLOR_YELLOW_B  "\x1b[33;1m"
#define ANSI_COLOR_BLUE_B    "\x1b[34;1m"
#define ANSI_COLOR_MAGENTA_B "\x1b[35;1m"
#define ANSI_COLOR_CYAN_B    "\x1b[36;1m"

#define ANSI_COLOR_RESET   "\x1b[0m"

/* For longopts */
#define _GNU_SOURCE
#include <getopt.h>

/* prototypes */
void  gen_error(char *message);                    /* Displays a general error */
void  f_error(char *filename, char *message);      /* Displays a general error involving a file */
int   dump_args(int argc, char *argv[]);           /* Aid for debugging */
char  *trim_whitespace(char *str);                 /* Removes leading and trailing whitespace from a string */
char  *path_alloc(int *sizep);                     /* Allocates memory for a pathname */
char  *file_perm_str(mode_t perm, int flags);      /* Displays a symbolic string of permission ie: rwxrw-rw- */
char  *filetype(mode_t st_mode, int flag);         /* Retuns the plain-english filetype from the stat struct */
int   file_perm_oct(mode_t perm);                  /* Returns octal representation of permissions */
char  *get_username(uid_t uid);                    /* Returns username from uid */
char  *get_groupname(gid_t gid);                   /* Returns groupname from gid */


/* Commonly used function definitions */

/* Generic error */
void gen_error(char *message) {
    printf("%s: %s\n", APPNAME, message);
}

/* File error */
void f_error(char *filename, char *message) {
    char error[50];
    snprintf(error, 50, "%s: %s %s", APPNAME, (message == NULL) ? "" : message, filename);
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

char *filetype(mode_t st_mode, int flag) {
    switch (st_mode & S_IFMT) {
        case S_IFBLK:
            return (flag == 1) ? (char *)"block device"     : (char *)"b" ;
        case S_IFCHR:
            return (flag == 1) ? (char *)"character device" : (char *)"c" ;
        case S_IFDIR:
            return (flag == 1) ? (char *)"directory"        : (char *)"d" ;
        case S_IFIFO:
            return (flag == 1) ? (char *)"FIFO/pipe"        : (char *)"p" ;
        case S_IFLNK:
            return (flag == 1) ? (char *)"symlink"          : (char *)"l" ;
        case S_IFREG:
            return (flag == 1) ? (char *)"regular file"     : (char *)"-" ;
        case S_IFSOCK:
            return (flag == 1) ? (char *)"socket"           : (char *)"s" ;
        default:
            return (flag == 1) ? (char *)"unknown"          : (char *)"?" ;
    }
}

char *get_username(uid_t uid) {
        struct passwd *pwd;
        errno = 0;
        pwd = getpwuid(uid);

        if (pwd == NULL) {
                if (errno == 0) {
                        return (char *)"unknown username";
                } else {
                        gen_error((char *)"username lookup failed");
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
            return (char *)"unknown groupname";
        } else {
            gen_error((char *)"groupname lookup failed");
            exit(EXIT_FAILURE);
        }
    }
    return grp->gr_name;
}

#endif /* _MY_COMMON_H */
