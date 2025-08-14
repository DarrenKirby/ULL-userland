/***************************************************************************
 *   mount.h - mount filesystems                                           *
 *                                                                         *
 *   Copyright (C) 2014 - 2024 by Darren Kirby                             *
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


#ifndef _MOUNT_H
#define _MOUNT_H

#include <stdio.h>
#include <sys/statfs.h>      /* for statfs struct */
#include <linux/limits.h>    /* for PATH_MAX */
#include <errno.h>           /* for perror() */
#include <stdlib.h>          /* for EXIT_FAILURE */
#include <string.h>          /* for strncpy */
#include <unistd.h>          /* for access() */
#include <sys/stat.h>        /* for stat   */
#include <sys/types.h>       /*  "    "    */

#define ERROR  -1
#define SUCCESS 0

#define FS_TYPE_LEN      90
#define MNT_FLAGS_LEN    256


struct mounted_fs_entry {
    char fs_spec[PATH_MAX];           /* device or special file system path */
    char fs_file[PATH_MAX];           /* mount point */
    char fs_vsftype[FS_TYPE_LEN];     /* file system type */
    char fs_mntops[MNT_FLAGS_LEN];    /* mount flags */
    int  fs_freq;                     /* dump */
    int  fs_passno;                   /* pass */
};


#if __WORDSIZE == 32
#define __WORD_TYPE int
#else /* __WORDSIZE == 64 */
#define __WORD_TYPE long int
#endif

struct statfs_ext {
    __WORD_TYPE  f_type;             /* type of filesystem */
    __WORD_TYPE  f_bsize;            /* optimal transfer block size */
    fsblkcnt_t   f_blocks;           /* total data blocks in filesystem */
    fsblkcnt_t   f_bfree;            /* free blocks in fs */
    fsblkcnt_t   f_bavail;           /* free blocks available to unprivileged user */
    fsfilcnt_t   f_files;            /* total file nodes in filesystem */
    fsfilcnt_t   f_ffree;            /* free file nodes in fs */
    __fsid_t     f_fsid;             /* filesystem id */
    __WORD_TYPE  f_namelen;          /* maximum length of filenames */
    __WORD_TYPE  f_frsize;           /* fragment size (since Linux 2.6) */
    __WORD_TYPE  f_spare[5];

    /* these extra fields add path info as in the *BSD versions of statfs() */
    char f_fstypename[FS_TYPE_LEN];  /* fs type name */
    char f_mntonname[PATH_MAX];      /* directory on which mounted */
    char f_mntfromname[PATH_MAX];    /* mounted file sytem */
};


/* These constants are for passing to the `bufsize` argument of
 * getfsstat_ext(). Passing FS_NUM will tell the function to just
 * return the number of mounted filesystems. FS_1 through FS_20 will
 * populate statfs_ext structs for the first 1-20 mounted filesystems.
 * FS_ALL will populate structs for all mounted filesystems.
 *
 * Note that getfsstat_ext's return value will be the number of mounted
 * filesystems it has found regardless of how many structs were asked for.
 */

#define STATFS_EXT_SIZE sizeof(struct statfs_ext)
#define FS_NUM 0
#define FS_1 STATFS_EXT_SIZE
#define FS_2 (FS_1 * 2)
#define FS_3 (FS_1 * 3)
#define FS_4 (FS_1 * 4)
#define FS_5 (FS_1 * 5)
#define FS_6 (FS_1 * 6)
#define FS_7 (FS_1 * 7)
#define FS_8 (FS_1 * 8)
#define FS_9 (FS_1 * 9)
#define FS_10 (FS_1 * 10)
#define FS_11 (FS_1 * 11)
#define FS_12 (FS_1 * 12)
#define FS_13 (FS_1 * 13)
#define FS_14 (FS_1 * 14)
#define FS_15 (FS_1 * 15)
#define FS_16 (FS_1 * 16)
#define FS_17 (FS_1 * 17)
#define FS_18 (FS_1 * 18)
#define FS_19 (FS_1 * 19)
#define FS_20 (FS_1 * 20)
// bufsize expects a long int, so this must be
// an arbitrary 'magic number'.
#define FS_ALL 0xdeadbee

/* used internally by statfs_ext() and getfsstat_ext() */
int __merge_statfs_structs(struct statfs *buf, struct statfs_ext *buf_full) {
    buf_full->f_type    = buf->f_type;
    buf_full->f_bsize   = buf->f_bsize;
    buf_full->f_blocks  = buf->f_blocks;
    buf_full->f_bfree   = buf->f_bfree;
    buf_full->f_bavail  = buf->f_bavail;
    buf_full->f_files   = buf->f_files;
    buf_full->f_ffree   = buf->f_ffree;
    buf_full->f_fsid    = buf->f_fsid;
    buf_full->f_namelen = buf->f_namelen;
    buf_full->f_frsize  = buf->f_frsize;

    for (int i = 0; i < 5; i++) {
        buf_full->f_spare[i] = buf->f_spare[i];
    }

    return SUCCESS;
}


/* used internally by statfs_ext() and getfsstat_ext() */
int __read_proc_mounts(struct mounted_fs_entry *mnt_fs_buf, const char *path) {
    FILE *fp;
    if ((fp = fopen("/proc/mounts", "r")) == NULL) {
        errno = EIO;
        return ERROR;
    }

    memset(mnt_fs_buf, 0, sizeof(struct mounted_fs_entry));

    char line_buf[512];
    char *line_buf_p = line_buf;
    char *tok;

    while (fgets(line_buf_p, 512, fp) != NULL) {
        if (strstr(line_buf_p, "rootfs") != NULL) {
            continue;  // Skip rootfs
        }

        // Tokenize the line
        tok = strtok(line_buf_p, " ");
        if (tok == NULL) continue;
        strncpy(mnt_fs_buf->fs_spec, tok, PATH_MAX - 1);

        tok = strtok(NULL, " ");
        if (tok == NULL) continue;
        strncpy(mnt_fs_buf->fs_file, tok, PATH_MAX - 1);

        tok = strtok(NULL, " ");
        if (tok == NULL) continue;
        strncpy(mnt_fs_buf->fs_vsftype, tok, FS_TYPE_LEN - 1);

        tok = strtok(NULL, " ");
        if (tok == NULL) continue;
        strncpy(mnt_fs_buf->fs_mntops, tok, MNT_FLAGS_LEN - 1);

        // We can use sscanf for the two integers at the end
        tok = strtok(NULL, " ");
        if (tok != NULL) {
            sscanf(tok, "%i", &mnt_fs_buf->fs_freq);
        }

        tok = strtok(NULL, " ");
        if (tok != NULL) {
            sscanf(tok, "%i", &mnt_fs_buf->fs_passno);
        }

        // Check if the mount point matches the desired path
        if (strcmp(mnt_fs_buf->fs_file, path) == 0) {
            fclose(fp);
            return SUCCESS;  // We found the matching entry
        }
    }

    fclose(fp);
    return ERROR;  // No matching mount point found
}


int getmntpt(const char *path, char *mount_point) {
    struct stat cur_stat;
    struct stat last_stat;

    char dir_name[PATH_MAX];
    char *dirname_p = dir_name;
    char cur_cwd[255];
    char *cur_cwd_p = cur_cwd;
    char saved_cwd[PATH_MAX];
    if (getcwd(saved_cwd, PATH_MAX) == NULL) {
        errno = EIO;
        return ERROR;
    }

    if (lstat(path, &cur_stat) < 0) {
        errno = EIO;
        return ERROR;
    }

    if (S_ISDIR (cur_stat.st_mode)) {
        last_stat = cur_stat;
        if (chdir("..") < 0)
            return ERROR;
        if (getcwd(cur_cwd_p, 255) == NULL) {
            errno = EIO;
            return ERROR;
        }
    } else { /* path is a file */
        size_t path_len, suffix_len, dir_len;
        path_len = strlen(path);
        suffix_len = strlen(strrchr(path, 47)); /* 47 = '/' */
        dir_len = path_len - suffix_len;
        dirname_p = strncpy(dirname_p, path, dir_len);
        if (chdir(dirname_p) < 0)
            return ERROR;
        if (lstat(".", &last_stat) < 0)
            return ERROR;
    }

    for (;;) {
        if (lstat("..", &cur_stat) < 0)
            return ERROR;
        if (cur_stat.st_dev != last_stat.st_dev || cur_stat.st_ino == last_stat.st_ino)
            break; /* this is the mount point */
        if (chdir("..") < 0)
            return ERROR;
        last_stat = cur_stat;
    }

    if (getcwd(mount_point, PATH_MAX) == NULL)
        return ERROR;
    if (chdir(saved_cwd) < 0)
        return ERROR;
    return SUCCESS;
}


int statfs_ext(const char *path, struct statfs_ext *struct_buf) {
    /* check size of path arg */
    if (strlen(path) > PATH_MAX) {
        errno = ENAMETOOLONG;
        return ERROR;
    }

    /* check if path exists */
    if (access(path, F_OK) != 0) {
        errno = ENOENT;
        return ERROR;
    }

    /* check if we have access to path */
    if (access(path, R_OK) != 0) {
        errno = EACCES;
        return ERROR;
    }

    char mount_path[PATH_MAX];
    char *mount_p = mount_path;

    if (getmntpt(path, mount_p) != 0)
        return ERROR;

    struct mounted_fs_entry mnt_fs_struct = {};
    struct statfs def_struct_tmp = {};

    __read_proc_mounts(&mnt_fs_struct, mount_p);

    if (statfs(mount_p, &def_struct_tmp) != 0)
        perror("statfs failed");

    __merge_statfs_structs(&def_struct_tmp, struct_buf);

    strncpy(struct_buf->f_fstypename, mnt_fs_struct.fs_vsftype, FS_TYPE_LEN);
    strncpy(struct_buf->f_mntonname, mnt_fs_struct.fs_file, PATH_MAX);
    strncpy(struct_buf->f_mntfromname, mnt_fs_struct.fs_spec, PATH_MAX);

    struct_buf->f_fstypename[FS_TYPE_LEN - 1] = '\0';
    struct_buf->f_mntonname[PATH_MAX - 1] = '\0';
    struct_buf->f_mntfromname[PATH_MAX - 1] = '\0';

    return SUCCESS;
}

int getfsstat_ext(struct statfs_ext **struct_array_buf, long int bufsize, int flags) {

    FILE *fp;
    if ((fp = fopen("/proc/mounts", "r")) == NULL) {
        errno = EIO;
        return ERROR;
    }

    char ch;
    int n_mounts = 0;
    while (!feof(fp)) {         /* count lines to determine */
        ch = fgetc(fp);         /* size of struct array     */
        if (ch == '\n')
            n_mounts++;
    }

    if (n_mounts <= 0) {
        fclose(fp);
        errno = EIO;
        return ERROR;
    }

    /* If the buffer size is FS_NUM, the user
     * only wants the number of mounts */
    if (bufsize == FS_NUM) {
        fclose(fp);
        return n_mounts;
    }

    /* Otherwise, allocate memory for the number
     * of requested mounted filesystems */
    int n_lines;
    if (bufsize == FS_ALL) {
        n_lines = n_mounts;
        *struct_array_buf = malloc(FS_1 * n_lines);
    } else {
        n_lines = bufsize / STATFS_EXT_SIZE;
        *struct_array_buf = malloc(bufsize);
    }
    if (*struct_array_buf == NULL) {
        errno = ENOMEM;
        return ERROR;
    }

    rewind(fp);

    struct mounted_fs_entry mounted_fs_struct[n_lines];
    int i = 0;
    char line_buf[256];
    char *line_buf_p = line_buf;
    for (; i < n_lines; i++) {
        line_buf_p = fgets(line_buf_p, sizeof(line_buf), fp);

        int parsed_fields = sscanf(line_buf_p, "%s %s %s %s %d %d\n",
                                   mounted_fs_struct[i].fs_spec, mounted_fs_struct[i].fs_file,
                                   mounted_fs_struct[i].fs_vsftype, mounted_fs_struct[i].fs_mntops,
                                   &mounted_fs_struct[i].fs_freq, &mounted_fs_struct[i].fs_passno);

        if (parsed_fields < 6) {
            fprintf(stderr, "Error parsing line: %s\n", line_buf_p);
            continue;
        }
    }
    fclose(fp);

    struct statfs_ext tmp_buf = {};

    for (i = 0; i < n_lines; i++) {

        if (statfs_ext(mounted_fs_struct[i].fs_file, &tmp_buf) != SUCCESS) {
            /* If statfs_ext fails, set default values */
            tmp_buf.f_type = 0;
            tmp_buf.f_bsize = 0;
            tmp_buf.f_blocks = 0;
            tmp_buf.f_bfree = 0;
            tmp_buf.f_bavail = 0;
            tmp_buf.f_files = 0;
            tmp_buf.f_ffree = 0;
            memset(&tmp_buf.f_fsid, 0, sizeof(tmp_buf.f_fsid));  // Set fsid to 0
            tmp_buf.f_namelen = 0;
            tmp_buf.f_frsize = 0;
            for (int j = 0; j < 5; j++) {
                tmp_buf.f_spare[j] = 0;
            }
        }

        /* Copy the known good values from mounted_fs_struct */
        strncpy(tmp_buf.f_fstypename, mounted_fs_struct[i].fs_vsftype, FS_TYPE_LEN - 1);
        strncpy(tmp_buf.f_mntonname, mounted_fs_struct[i].fs_file, PATH_MAX - 1);
        strncpy(tmp_buf.f_mntfromname, mounted_fs_struct[i].fs_spec, PATH_MAX - 1);

        /* Copy the result from tmp_buf into the array */
        (*struct_array_buf)[i] = tmp_buf;
    }

    return n_lines; /* number of mounted filesystems */
}



// #ifndef _XOPEN_SOURCE
// #define _XOPEN_SOURCE 700
// #endif
//
// #ifdef __linux__
// #include <sys/stat.h>
// #include <sys/statfs.h>      /* for statfs struct */
// #include <linux/limits.h>    /* for PATH_MAX */
// #else
// #include <sys/param.h>
// #include <sys/mount.h>
// #endif
// #include <errno.h>           /* for perror() */
// #include <stdlib.h>          /* for EXIT_FAILURE */
// #include <string.h>
// #include <sys/types.h>
//
// #include "common.h"
//
// //typedef unsigned long int fsid_t;
// //typedef struct { int val[2]; } fsid_t;
//
// #define FS_TYPE_LEN      90
// #define MNT_FLAGS_LEN    256
//
// struct mounted_fs_entry {
//     char fs_spec[PATH_MAX+1];         /* device or special file system path */
//     char fs_file[PATH_MAX+1];         /* mount point */
//     char fs_vsftype[FS_TYPE_LEN];     /* file system type */
//     char fs_mntops[MNT_FLAGS_LEN];    /* mount flags */
//     int  fs_freq;                     /* dump */
//     int  fs_passno;                   /* pass */
// };
//
// #if __WORDSIZE == 32
// #define __WORD_TYPE int
// #else /* __WORDSIZE == 64 */
// #define __WORD_TYPE long int
// #endif
//
// struct statfs_ext {
//     __WORD_TYPE  f_type;             /* type of filesystem (see below) */
//     __WORD_TYPE  f_bsize;            /* optimal transfer block size */
//     fsblkcnt_t   f_blocks;           /* total data blocks in filesystem */
//     fsblkcnt_t   f_bfree;            /* free blocks in fs */
//     fsblkcnt_t   f_bavail;           /* free blocks available to unprivileged user */
//     fsfilcnt_t   f_files;            /* total file nodes in filesystem */
//     fsfilcnt_t   f_ffree;            /* free file nodes in fs */
//     __fsid_t     f_fsid;             /* filesystem id */
//     __WORD_TYPE  f_namelen;          /* maximum length of filenames */
//     __WORD_TYPE  f_frsize;           /* fragment size (since Linux 2.6) */
//     __WORD_TYPE  f_spare[5];
//
//     /* these extra fields add path info as in the *BSD versions of statfs() */
//     char f_fstypename[FS_TYPE_LEN+1];  /* fs type name */
//     char f_mntonname[PATH_MAX+1];      /* directory on which mounted */
//     char f_mntfromname[PATH_MAX+1];    /* mounted file sytem */
// };
//
int merge_statfs_structs(struct statfs *buf, struct statfs_ext **buf_full) {
    int i;
    (*buf_full)->f_type    = buf->f_type;
    (*buf_full)->f_bsize   = buf->f_bsize;
    (*buf_full)->f_blocks  = buf->f_blocks;
    (*buf_full)->f_bfree   = buf->f_bfree;
    (*buf_full)->f_bavail  = buf->f_bavail;
    (*buf_full)->f_files   = buf->f_files;
    (*buf_full)->f_ffree   = buf->f_ffree;
    (*buf_full)->f_fsid    = buf->f_fsid;
    (*buf_full)->f_namelen = buf->f_namelen;
    (*buf_full)->f_frsize  = buf->f_frsize;

    for (i = 0; i < 5; i++)
        (*buf_full)->f_spare[i]   = buf->f_spare[i];
    return 0;
}
//
// int statfs_ext(const char *path, struct statfs_ext *struct_buf) {
//     /* check size of path arg */
//     if (strlen(path) > PATH_MAX) {
//         errno = ENAMETOOLONG;
//         return ERROR;
//     }
//
//     /* check if path exists */
//     if (access(path, F_OK) != 0) {
//         errno = ENOENT;
//         return ERROR;
//     }
//
//     /* check if we have access to path */
//     if (access(path, R_OK) != 0) {
//         errno = EACCES;
//         return ERROR;
//     }
//
//     char mount_path[PATH_MAX];
//     char *mount_p = mount_path;
//
//     if (getmntpt(path, mount_p) != 0)
//         return ERROR;
//
//     struct mounted_fs_entry mnt_fs_struct = {};
//     struct statfs def_struct_tmp = {};
//
//     __read_proc_mounts(&mnt_fs_struct, mount_p);
//
//     if (statfs(mount_p, &def_struct_tmp) != 0)
//         perror("statfs failed");
//
//     __merge_statfs_structs(&def_struct_tmp, struct_buf);
//
//     strncpy(struct_buf->f_fstypename, mnt_fs_struct.fs_vsftype, FS_TYPE_LEN);
//     strncpy(struct_buf->f_mntonname, mnt_fs_struct.fs_file, PATH_MAX);
//     strncpy(struct_buf->f_mntfromname, mnt_fs_struct.fs_spec, PATH_MAX);
//
//     struct_buf->f_fstypename[FS_TYPE_LEN - 1] = '\0';
//     struct_buf->f_mntonname[PATH_MAX - 1] = '\0';
//     struct_buf->f_mntfromname[PATH_MAX - 1] = '\0';
//
//     return SUCCESS;
// }
//
// int getfsstat_ext(struct statfs_ext **struct_array_buf, long int bufsize, int flags) {
//
//     FILE *fp;
//     if ((fp = fopen("/proc/mounts", "r")) == NULL) {
//         errno = EIO;
//         return ERROR;
//     }
//
//     char ch;
//     int n_mounts = 0;
//     while (!feof(fp)) {         /* count lines to determine */
//         ch = fgetc(fp);         /* size of struct array     */
//         if (ch == '\n')
//             n_mounts++;
//     }
//
//     if (n_mounts <= 0) {
//         fclose(fp);
//         errno = EIO;
//         return ERROR;
//     }
//
//     /* If the buffer size is FS_NUM, the user
//      * only wants the number of mounts */
//     if (bufsize == FS_NUM) {
//         fclose(fp);
//         return n_mounts;
//     }
//
//     /* Otherwise, allocate memory for the number
//      * of requested mounted filesystems */
//     int n_lines;
//     if (bufsize == FS_ALL) {
//         n_lines = n_mounts;
//         *struct_array_buf = malloc(FS_1 * n_lines);
//     } else {
//         n_lines = bufsize / STATFS_EXT_SIZE;
//         *struct_array_buf = malloc(bufsize);
//     }
//     if (*struct_array_buf == NULL) {
//         errno = ENOMEM;
//         return ERROR;
//     }
//
//     rewind(fp);
//
//     struct mounted_fs_entry mounted_fs_struct[n_lines];
//     int i = 0;
//     char line_buf[256];
//     char *line_buf_p = line_buf;
//     for (; i < n_lines; i++) {
//         line_buf_p = fgets(line_buf_p, sizeof(line_buf), fp);
//
//         int parsed_fields = sscanf(line_buf_p, "%s %s %s %s %d %d\n",
//                                    mounted_fs_struct[i].fs_spec, mounted_fs_struct[i].fs_file,
//                                    mounted_fs_struct[i].fs_vsftype, mounted_fs_struct[i].fs_mntops,
//                                    &mounted_fs_struct[i].fs_freq, &mounted_fs_struct[i].fs_passno);
//
//         if (parsed_fields < 6) {
//             fprintf(stderr, "Error parsing line: %s\n", line_buf_p);
//             continue;
//         }
//     }
//     fclose(fp);
//
//     struct statfs_ext tmp_buf = {};
//
//     for (i = 0; i < n_lines; i++) {
//
//         if (statfs_ext(mounted_fs_struct[i].fs_file, &tmp_buf) != SUCCESS) {
//             /* If statfs_ext fails, set default values */
//             tmp_buf.f_type = 0;
//             tmp_buf.f_bsize = 0;
//             tmp_buf.f_blocks = 0;
//             tmp_buf.f_bfree = 0;
//             tmp_buf.f_bavail = 0;
//             tmp_buf.f_files = 0;
//             tmp_buf.f_ffree = 0;
//             memset(&tmp_buf.f_fsid, 0, sizeof(tmp_buf.f_fsid));  // Set fsid to 0
//             tmp_buf.f_namelen = 0;
//             tmp_buf.f_frsize = 0;
//             for (int j = 0; j < 5; j++) {
//                 tmp_buf.f_spare[j] = 0;
//             }
//         }
//
//         /* Copy the known good values from mounted_fs_struct */
//         strncpy(tmp_buf.f_fstypename, mounted_fs_struct[i].fs_vsftype, FS_TYPE_LEN - 1);
//         strncpy(tmp_buf.f_mntonname, mounted_fs_struct[i].fs_file, PATH_MAX - 1);
//         strncpy(tmp_buf.f_mntfromname, mounted_fs_struct[i].fs_spec, PATH_MAX - 1);
//
//         /* Copy the result from tmp_buf into the array */
//         (*struct_array_buf)[i] = tmp_buf;
//     }
//
//     return n_lines; /* number of mounted filesystems */
// }
//


int getfsstat_linux(struct statfs_ext *buf) {
    FILE *fp;
    int  n_lines = 0;
    char lines[512];
    char *line = lines;

    if ((fp = fopen("/proc/mounts", "r")) == NULL) {
        perror("could not open /proc/mounts");
        exit(EXIT_FAILURE);
    }

    char ch;
    while(!feof(fp)) {             /* count lines to determine */
        ch = fgetc(fp);            /* size of struct array     */
        if(ch == '\n')
            n_lines++;
    }
    rewind(fp);

    if (buf == NULL)               /* We have # of mounted fs, might as well bail */
        return n_lines;

    struct mounted_fs_entry fse[n_lines];
    int i, i2;

    for (i = 0; i < n_lines; i++) {
        line = fgets(lines, 512, fp);
        sscanf(line, "%s %s %s %s %d %d\n", fse[i].fs_spec, fse[i].fs_file,
                                            fse[i].fs_vsftype, fse[i].fs_mntops,
                                            &fse[i].fs_freq, &fse[i].fs_passno);
    }

    fclose(fp);

    /* statfs array struct */
    struct statfs_ext *sas = malloc(sizeof(struct statfs_ext) * n_lines);
    //struct statfs_ext *buf = malloc(sizeof(struct statfs_ext) * n_lines);

    struct statfs_ext *f_tmp;
    f_tmp = malloc(sizeof(struct statfs_ext));
    if (f_tmp == NULL) {
        perror("unable to malloc");
        exit(EXIT_FAILURE);
    }

    struct statfs *s_tmp;
    s_tmp = malloc(sizeof(struct statfs));
    if (s_tmp == NULL) {
        perror("unable to malloc");
        exit(EXIT_FAILURE);
    }

    for (i2 = 0; i2 < n_lines; i2++) {

        if (statfs(fse[i2].fs_file, s_tmp) != 0) {
            perror("statfs() failed");
            continue; /* might not be fatal */
        }
        merge_statfs_structs(s_tmp, &f_tmp);

        strncpy(f_tmp->f_fstypename, fse[i2].fs_vsftype, FS_TYPE_LEN);
        strncpy(f_tmp->f_mntonname, fse[i2].fs_file, PATH_MAX);
        strncpy(f_tmp->f_mntfromname, fse[i2].fs_spec, PATH_MAX);

        //printf("f_type:    %#x\n", (unsigned int)f_tmp->f_type);
        //printf("f_bsize:   %lu\n", f_tmp->f_bsize);
        //printf("f_blocks:  %d\n", (int)f_tmp->f_blocks);
        //printf("f_bfree:   %d\n", (int)f_tmp->f_bfree);
        //printf("f_bavail:  %d\n", (int)f_tmp->f_bavail);
        //printf("f_files:   %d\n", (int)f_tmp->f_files);
        //printf("f_ffree:   %d\n", (int)f_tmp->f_ffree);
        //printf("f_fsid1:   %lu\n", f_tmp->f_fsid);
        //printf("f_namelen  %lu\n", f_tmp->f_namelen);
        //printf("f_frsize   %lu\n", f_tmp->f_frsize);
        //printf("f_fstypename:  %s\n", f_tmp->f_fstypename);
        //printf("f_mntonname:   %s\n", f_tmp->f_mntonname);
        //printf("f_mntfromname: %s\n", f_tmp->f_mntfromname);

        sas[i2] = *f_tmp;

        //printf("f_type:        %#x\n", (unsigned int)sas[i2].f_type);
        //printf("f_bsize:       %lu\n", sas[i2].f_bsize);
        //printf("f_blocks:      %d\n",  (int)sas[i2].f_blocks);
        //printf("f_bfree:       %d\n",  (int)sas[i2].f_bfree);
        //printf("f_bavail:      %d\n",  (int)sas[i2].f_bavail);
        //printf("f_files:       %d\n",  (int)sas[i2].f_files);
        //printf("f_ffree:       %d\n",  (int)sas[i2].f_ffree);
        //printf("f_fsid1:     %lu\n", f_tmp->f_fsid);
        //printf("f_namelen      %lu\n", sas[i2].f_namelen);
        //printf("f_frsize       %lu\n", sas[i2].f_frsize);
        //printf("f_fstypename:  %s\n",  sas[i2].f_fstypename);
        //printf("f_mntonname:   %s\n",  sas[i2].f_mntonname);
        //printf("f_mntfromname: %s\n",  sas[i2].f_mntfromname);
    }

    free(s_tmp);
    free(f_tmp);
    //printf("f_fstypename:  %s\n", sas[0].f_fstypename);
    //printf("f_fstypename:  %s\n", sas[1].f_fstypename);
    //printf("f_fstypename:  %s\n", sas[2].f_fstypename);
    //printf("f_fstypename:  %s\n", sas[3].f_fstypename);
    //printf("f_fstypename:  %s\n", sas[4].f_fstypename);
    //printf("f_fstypename:  %s\n", sas[5].f_fstypename);
    //printf("f_fstypename:  %s\n", sas[6].f_fstypename);
    //printf("f_fstypename:  %s\n", sas[7].f_fstypename);
    //printf("f_fstypename:  %s\n", sas[8].f_fstypename);
    //printf("f_fstypename:  %s\n", sas[9].f_fstypename);
    //printf("Size of sas:   %lu\n", sizeof(sas));
    //printf("Size of buf:   %lu\n", sizeof(buf));
    //buf = **sas;
    free(sas);
    return i2;
}

#endif /* _MOUNT_H */

