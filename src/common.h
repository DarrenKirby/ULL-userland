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
#define APP_VERSION "0.4.4"

/* For OS X */
#if defined(__APPLE__) && defined(__MACH__)
#define st_atim      st_atimespec
#define st_mtim      st_mtimespec
#define st_ctim      st_ctimespec
#define st_birthtim  st_birthtimespec
#endif

/* Determine portable, local max path length. */
extern inline size_t get_path_max()
{
    return pathconf(".", _PC_PATH_MAX);
}

/* Determine portable, local max filename length. */
extern inline size_t get_filename_max()
{
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
extern inline void format_ls(const long long int bytes)
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

extern inline long parse_numeric_arg(char *arg, const int *min, const int *max, const char *name) {
    char *end_ptr;
    errno = 0;

    const long conv = strtol(arg, &end_ptr, 0);

    if (errno == ERANGE) {
        fprintf(stderr, "%s: arg  %s out of range\n", name, arg);
        exit(EXIT_FAILURE);
    }

    if (min) {
        if (conv < *min) {
            fprintf(stderr, "%s: arg %s below minimum of %d\n", name, arg, *min);
            exit(EXIT_FAILURE);
        }
    }

    if (max) {
        if (conv > *max) {
            fprintf(stderr, "%s: arg %s above maximum of %d\n", name, arg, *max);
            exit(EXIT_FAILURE);
        }
    }

    if (end_ptr == arg) {
        fprintf(stderr, "%s: argument is not parsable as a number\n", name);
        exit(EXIT_FAILURE);
    }

    return conv;
}

/* Debugging aids */
inline int dump_args(int argc, char *argv[])
{
  printf("argc: %i\n", argc);
    for (int i = 1; i < argc; i++) {
        printf("argv[%i]: %s\n", i, argv[i]);
    }
    return 0;
}

/* trims leading and tailing whitespace from strings */
extern inline char* trim_whitespace(char *str)
{
    size_t len = 0;
    const char *front_p = str;
    char *end_p = nullptr;

    if (!str) { return nullptr; }
    if (str[0] == '\0') { return str; }

    len = strlen(str);
    end_p = str + len;

    /* Move the front and back pointers to address the first non-whitespace
     * characters from each end. */
    while (isspace(*front_p)) { ++front_p; }
    if (end_p != front_p) {
        while (isspace(*(--end_p)) && end_p != front_p) {}
    }

    if (str + len - 1 != end_p) {
        *(end_p + 1) = '\0';
    } else if (front_p != str && end_p == front_p) {
        *str = '\0';
    }

    /* Shift the string so that it starts at str so that if it's dynamically
     * allocated, we can still free it on the returned pointer.  Note the reuse
     * of end_p to mean the front of the string buffer now. */
    end_p = str;
    if (front_p != str) {
        while (*front_p) { *end_p++ = *front_p++; }
        *end_p = '\0';
    }
    return str;
}

#define FP_SPECIAL 1
/* Include set-user-ID, set-group-ID, and sticky
bit information in returned string. */

#define PERM_STR_SIZE sizeof("rwxrwxrwx")

/* Return 'ls -l' style string for file permissions mask, This is from
 * 'The Linux Programming Interface'. */
extern inline char* file_perm_str(const mode_t perm, const int flags)
{
    char *str = malloc(PERM_STR_SIZE + 1);
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

/* Returns octal permissions of a file/directory. */
extern inline int file_perm_oct(const mode_t perm) {
    int oct_perm = 00;
    if (perm & S_ISUID) oct_perm += 04000;
    if (perm & S_ISGID) oct_perm += 02000;
    if (perm & S_ISVTX) oct_perm += 01000;
    if (perm & S_IRUSR) oct_perm += 0400;
    if (perm & S_IWUSR) oct_perm += 0200;
    if (perm & S_IXUSR) oct_perm += 0100;
    if (perm & S_IRGRP) oct_perm += 040;
    if (perm & S_IWGRP) oct_perm += 020;
    if (perm & S_IXGRP) oct_perm += 010;
    if (perm & S_IROTH) oct_perm += 04;
    if (perm & S_IWOTH) oct_perm += 02;
    if (perm & S_IXOTH) oct_perm += 01;
    return oct_perm;
}

extern inline char* filetype(const mode_t st_mode, const bool long_form)
{
    switch (st_mode & S_IFMT) {
    case S_IFBLK:
        return long_form ? "block device" : "b";
    case S_IFCHR:
        return long_form ? "character device" : "c";
    case S_IFDIR:
        return long_form ? "directory" : "d";
    case S_IFIFO:
        return long_form ? "FIFO/pipe" : "p";
    case S_IFLNK:
        return long_form ? "symlink" : "l";
    case S_IFREG:
        return long_form ? "regular file" : "-";
    case S_IFSOCK:
        return long_form ? "socket" : "s";
    default:
        return long_form ? "unknown" : "?";
    }
}

extern inline char* get_username(const uid_t uid)
{
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

extern inline char* get_groupname(const gid_t gid)
{
    errno = 0;
    const struct group *grp = getgrgid(gid);
    if (grp == NULL) {
        if (errno == 0) {
            return (char *)"unknown group name";
        }
        fprintf(stderr, "group name lookup failed");
        exit(EXIT_FAILURE);
    }
    return grp->gr_name;
}

#endif /* COMMON_H */
