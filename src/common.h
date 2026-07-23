/***************************************************************************
 *   common.h - includes and functions common to all files                 *
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

#ifndef COMMON_H
#define COMMON_H

#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <ctype.h>
#include <pwd.h>
#include <grp.h>


/* Version information */
#define APP_SUITE   "ULL-userland"
#define APP_VERSION "0.4.1"

/* For OS X */
#if defined(__APPLE__) && defined(__MACH__)
#define st_atim      st_atimespec
#define st_mtim      st_mtimespec
#define st_ctim      st_ctimespec
#define st_birthtim  st_birthtimespec
#endif

/* Determine portable, local max path length. */
extern inline size_t get_path_max() {
    return pathconf(".", _PC_PATH_MAX);
}

/* Determine portable, local max filename length. */
extern inline size_t get_filename_max() {
    return pathconf(".", _PC_NAME_MAX);
}

/*
 * ANSI colour codes
 */

/* normal */
#define ANSI_RED       "\x1b[31m"
#define ANSI_GREEN     "\x1b[32m"
#define ANSI_YELLOW    "\x1b[33m"
#define ANSI_BLUE      "\x1b[34m"
#define ANSI_MAGENTA   "\x1b[35m"
#define ANSI_CYAN      "\x1b[36m"

/* bold */
#define ANSI_RED_B     "\x1b[31;1m"
#define ANSI_GREEN_B   "\x1b[32;1m"
#define ANSI_YELLOW_B  "\x1b[33;1m"
#define ANSI_BLUE_B    "\x1b[34;1m"
#define ANSI_MAGENTA_B "\x1b[35;1m"
#define ANSI_CYAN_B    "\x1b[36;1m"

#define ANSI_RESET     "\x1b[0m"

/* Formats 'human readable' output.
 * Used by ls and vdir. */
extern inline void format(const long long int bytes)
{
    char size_string[22];
    double result;
    if (bytes < 1024) {
        if (snprintf(size_string, 21, "%lld", bytes) < 0) {
            perror("sprintf"); exit(EXIT_FAILURE);
        }
    } else if ((bytes > 1025) && (bytes <= 1025000)) {
        result = (double)bytes / 1024.0;
        if (snprintf(size_string, 21, "%5.1fK", result) < 0) {
            perror("sprintf"); exit(EXIT_FAILURE);
        }
    } else if ((bytes > 1025000) && (bytes <= 1025000000)) {
        result = (double)bytes / 1024.0 / 1024.0;
        if (snprintf(size_string, 21, "%5.1fM", result) < 0) {
            perror("sprintf"); exit(EXIT_FAILURE);
        }
    } else {
        result = (double)bytes / 1024.0 / 1024.0 / 1024.0;
        if (snprintf(size_string, 21, "%5.1fG", result) < 0) {
            perror("sprintf"); exit(EXIT_FAILURE);
        }
    }
    printf("%6s ", size_string);
}

/* Debugging aids */
inline int dump_args(int argc, char *argv[]) {
  printf("argc: %i\n", argc);
    for (int i = 1; i < argc; i++) {
        printf("argv[%i]: %s\n", i, argv[i]);
    }
    return 0;
}

/* trims leading and tailing whitespace from strings */
inline char *trim_whitespace(char *str) {
    size_t len = 0;
    char *frontp = str;
    char *endp = nullptr;

    if( str == NULL ) { return nullptr; }
    if( str[0] == '\0' ) { return str; }

    len = strlen(str);
    endp = str + len;

    /* Move the front and back pointers to address the first non-whitespace
     * characters from each end. */
    while (isspace(*frontp)) { ++frontp; }
    if (endp != frontp) {
        while (isspace(*(--endp)) && endp != frontp) {}
    }

    if (str + len - 1 != endp) {
        *(endp + 1) = '\0';
    } else if (frontp != str &&  endp == frontp) {
        *str = '\0';
    }

    /* Shift the string so that it starts at str so that if it's dynamically
     * allocated, we can still free it on the returned pointer.  Note the reuse
     * of endp to mean the front of the string buffer now. */
    endp = str;
    if (frontp != str) {
        while (*frontp) { *endp++ = *frontp++; }
        *endp = '\0';
    }
    return str;
}

#define FP_SPECIAL 1
/* Include set-user-ID, set-group-ID, and sticky
bit information in returned string */

#define PERM_STR_SIZE sizeof("rwxrwxrwx")

/* Return 'ls -l' style string for file permissions mask, This is from
 * 'The Linux Programming Interface' */
extern inline char *file_perm_str(const mode_t perm, const int flags) {
    char *str = malloc(PERM_STR_SIZE + 1); //[PERM_STR_SIZE];
    if (!str) {
        fprintf(stderr, "malloc failed\n");
        exit(EXIT_FAILURE);
    }
    snprintf(str, PERM_STR_SIZE, "%c%c%c%c%c%c%c%c%c",
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
extern inline int file_perm_oct(const mode_t perm) {
    int oct_perm = 00;
    (perm & S_ISUID) ? (oct_perm += 04000) : (oct_perm += 00);
    (perm & S_ISGID) ? (oct_perm += 02000) : (oct_perm += 00);
    (perm & S_ISVTX) ? (oct_perm += 01000) : (oct_perm += 00);
    (perm & S_IRUSR) ? (oct_perm += 0400)  : (oct_perm += 00);
    (perm & S_IWUSR) ? (oct_perm += 0200)  : (oct_perm += 00);
    (perm & S_IXUSR) ? (oct_perm += 0100)  : (oct_perm += 00);
    (perm & S_IRGRP) ? (oct_perm += 040)   : (oct_perm += 00);
    (perm & S_IWGRP) ? (oct_perm += 020)   : (oct_perm += 00);
    (perm & S_IXGRP) ? (oct_perm += 010)   : (oct_perm += 00);
    (perm & S_IROTH) ? (oct_perm += 04)    : (oct_perm += 00);
    (perm & S_IWOTH) ? (oct_perm += 02)    : (oct_perm += 00);
    (perm & S_IXOTH) ? (oct_perm += 01)    : (oct_perm += 00);
    return oct_perm;
}

extern inline char *filetype(const mode_t st_mode, const int flag) {
    switch (st_mode & S_IFMT) {
    case S_IFBLK:
        return (flag == 1) ? (char *)"block device" : (char *)"b";
    case S_IFCHR:
        return (flag == 1) ? (char *)"character device" : (char *)"c";
    case S_IFDIR:
        return (flag == 1) ? (char *)"directory" : (char *)"d";
    case S_IFIFO:
        return (flag == 1) ? (char *)"FIFO/pipe" : (char *)"p";
    case S_IFLNK:
        return (flag == 1) ? (char *)"symlink" : (char *)"l";
    case S_IFREG:
        return (flag == 1) ? (char *)"regular file" : (char *)"-";
    case S_IFSOCK:
        return (flag == 1) ? (char *)"socket" : (char *)"s";
    default:
        return (flag == 1) ? (char *)"unknown" : (char *)"?";
    }
}

extern inline char *get_username(const uid_t uid) {
    errno = 0;
    const struct passwd *pwd = getpwuid(uid);

    if (pwd == NULL) {
        if (errno == 0) {
            return (char *)"unknown username";
        }
        fprintf(stderr, "username lookup failed");
        exit(EXIT_FAILURE);
    }
    return pwd->pw_name;
}

extern inline char *get_groupname(const gid_t gid) {
    errno = 0;
    const struct group *grp = getgrgid(gid);
    if (grp == NULL) {
        if (errno == 0) {
            return (char *)"unknown groupname";
        }
        fprintf(stderr, "groupname lookup failed");
        exit(EXIT_FAILURE);
    }
    return grp->gr_name;
}

#endif /* COMMON_H */
