/***************************************************************************
 *   mount.h - mount filesystems                                           *
 *                                                                         *
 *   Copyright (C) 2014 - 2026 by Darren Kirby                             *
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

#ifndef MOUNT_H
#define MOUNT_H

#include <stdio.h>
#ifdef __linux__
#include <sys/vfs.h>      /* for statfs struct */
#include <linux/limits.h>    /* for PATH_MAX */
#endif
#include <errno.h>           /* for perror() */
#include <stdlib.h>          /* for EXIT_FAILURE */
#include <string.h>          /* for strncpy */
#include <unistd.h>          /* for access() */
#include <sys/stat.h>        /* for stat   */
#include <sys/types.h>       /*  "    "    */

#define FS_TYPE_LEN      90
#define MNT_FLAGS_LEN    256
#define RETURN_ERR (-1)


struct mounted_fs_entry {
    char fs_spec[PATH_MAX];           /* device or special file system path */
    char fs_file[PATH_MAX];           /* mount point */
    char fs_vsftype[FS_TYPE_LEN];     /* file system type */
    char fs_mntops[MNT_FLAGS_LEN];    /* mount flags */
    int  fs_freq;                     /* dump */
    int  fs_passno;                   /* pass */
};

struct statfs_ext {
    uint32_t     f_type;             /* type of filesystem */
    uint32_t     f_bsize;            /* optimal transfer block size */
    fsblkcnt_t   f_blocks;           /* total data blocks in filesystem */
    fsblkcnt_t   f_bfree;            /* free blocks in fs */
    fsblkcnt_t   f_bavail;           /* free blocks available to unprivileged user */
    fsfilcnt_t   f_files;            /* total file nodes in filesystem */
    fsfilcnt_t   f_ffree;            /* free file nodes in fs */
    __fsid_t     f_fsid;             /* filesystem id */
    uint32_t     f_namelen;          /* maximum length of filenames */
    uint32_t     f_frsize;           /* fragment size (since Linux 2.6) */
    uint32_t     f_spare[5];

    /* these extra fields add path info as in the *BSD versions of statfs() */
    char f_fstypename[FS_TYPE_LEN];  /* fs type name */
    char f_mntonname[PATH_MAX];      /* directory on which mounted */
    char f_mntfromname[PATH_MAX];    /* mounted file sytem */
};

/* used internally by statfs_ext() and getfsstat_ext() */
static int merge_statfs_structs(const struct statfs *buf, struct statfs_ext *buf_full) {
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

    for (int i = 0; i < 4; i++) {
        buf_full->f_spare[i] = buf->f_spare[i];
    }

    return EXIT_SUCCESS;
}


/* used internally by statfs_ext() and getfsstat_ext() */
static int read_proc_mounts(struct mounted_fs_entry *mnt_fs_buf, const char *path) {
    FILE *fp;
    if ((fp = fopen("/proc/mounts", "r")) == NULL) {
        errno = EIO;
        return RETURN_ERR;
    }

    memset(mnt_fs_buf, 0, sizeof(struct mounted_fs_entry));

    char line_buf[512];
    char *line_buf_p = line_buf;

    while (fgets(line_buf_p, 512, fp) != NULL) {
        if (strstr(line_buf_p, "rootfs") != NULL) {
            continue;  // Skip rootfs
        }

        /* Tokenize the line. */
        char *tok = strtok(line_buf_p, " ");
        if (tok == NULL) continue;
        strncpy(mnt_fs_buf->fs_spec, tok, PATH_MAX - 1);

        tok = strtok(nullptr, " ");
        if (tok == NULL) continue;
        strncpy(mnt_fs_buf->fs_file, tok, PATH_MAX - 1);

        tok = strtok(nullptr, " ");
        if (tok == NULL) continue;
        strncpy(mnt_fs_buf->fs_vsftype, tok, FS_TYPE_LEN - 1);

        tok = strtok(nullptr, " ");
        if (!tok) continue;
        strncpy(mnt_fs_buf->fs_mntops, tok, MNT_FLAGS_LEN - 1);

        // We can use sscanf for the two integers at the end
        tok = strtok(nullptr, " ");
        if (!tok) {
            sscanf(tok, "%i", &mnt_fs_buf->fs_freq);
        }

        tok = strtok(nullptr, " ");
        if (!tok) {
            sscanf(tok, "%i", &mnt_fs_buf->fs_passno);
        }

        // Check if the mount point matches the desired path
        if (strcmp(mnt_fs_buf->fs_file, path) == 0) {
            fclose(fp);
            return EXIT_SUCCESS;  // We found the matching entry
        }
    }

    fclose(fp);
    return RETURN_ERR;  // No matching mount point found
}


static int getmntpt(const char *path, char *mount_point) {
    struct stat cur_stat;
    struct stat last_stat;

    char dir_name[PATH_MAX];
    char *dirname_p = dir_name;
    char cur_cwd[PATH_MAX];
    char *cur_cwd_p = cur_cwd;
    char saved_cwd[PATH_MAX];
    if (getcwd(saved_cwd, PATH_MAX) == NULL) {
        errno = EIO;
        return RETURN_ERR;
    }

    if (lstat(path, &cur_stat) < 0) {
        errno = EIO;
        return RETURN_ERR;
    }

    if (S_ISDIR (cur_stat.st_mode)) {
        last_stat = cur_stat;
        if (chdir("..") < 0)
            return RETURN_ERR;
        if (getcwd(cur_cwd_p, PATH_MAX) == NULL) {
            errno = EIO;
            return RETURN_ERR;
        }
    } else { /* path is a file */
        const size_t path_len = strlen(path) + 1;
        const size_t suffix_len = strlen(strrchr(path, 47)); /* 47 = '/' */
        const size_t dir_len = path_len - suffix_len;
        memcpy(dirname_p, path, dir_len);

        if (chdir(dirname_p) < 0)
            return RETURN_ERR;
        if (lstat(".", &last_stat) < 0)
            return RETURN_ERR;
    }

    for (;;) {
        if (lstat("..", &cur_stat) < 0)
            return RETURN_ERR;
        if (cur_stat.st_dev != last_stat.st_dev || cur_stat.st_ino == last_stat.st_ino)
            break; /* this is the mount point */
        if (chdir("..") < 0)
            return RETURN_ERR;
        last_stat = cur_stat;
    }

    if (getcwd(mount_point, PATH_MAX) == NULL)
        return RETURN_ERR;
    if (chdir(saved_cwd) < 0)
        return RETURN_ERR;
    return EXIT_SUCCESS;
}

static int statfs_ext(const char *path, struct statfs_ext *struct_buf) {
    /* check size of path arg */
    if (strlen(path) > PATH_MAX) {
        errno = ENAMETOOLONG;
        return RETURN_ERR;
    }

    /* check if path exists */
    if (access(path, F_OK) != 0) {
        errno = ENOENT;
        return RETURN_ERR;
    }

    /* check if we have access to path */
    if (access(path, R_OK) != 0) {
        errno = EACCES;
        return RETURN_ERR;
    }

    char mount_path[PATH_MAX];
    char *mount_p = mount_path;

    if (getmntpt(path, mount_p) != 0)
        return RETURN_ERR;

    struct mounted_fs_entry mnt_fs_struct = {};
    struct statfs def_struct_tmp = {};

    read_proc_mounts(&mnt_fs_struct, mount_p);

    if (statfs(mount_p, &def_struct_tmp) != 0)
        perror("statfs failed");

    merge_statfs_structs(&def_struct_tmp, struct_buf);

    snprintf(struct_buf->f_fstypename, FS_TYPE_LEN, "%s", mnt_fs_struct.fs_vsftype);
    snprintf(struct_buf->f_mntonname, PATH_MAX, "%s", mnt_fs_struct.fs_file);
    snprintf(struct_buf->f_mntfromname, PATH_MAX, "%s", mnt_fs_struct.fs_spec);

    // strncpy(struct_buf->f_fstypename, mnt_fs_struct.fs_vsftype, FS_TYPE_LEN - 1);
    // strncpy(struct_buf->f_mntonname, mnt_fs_struct.fs_file, PATH_MAX - 1);
    // strncpy(struct_buf->f_mntfromname, mnt_fs_struct.fs_spec, PATH_MAX - 1);
    //
    // struct_buf->f_fstypename[FS_TYPE_LEN - 1] = '\0';
    // struct_buf->f_mntonname[PATH_MAX - 1] = '\0';
    // struct_buf->f_mntfromname[PATH_MAX - 1] = '\0';

    return EXIT_SUCCESS;
}

static int getfsstat_ext(struct statfs_ext *struct_array_buf, const size_t bufsize) {

    FILE *fp;
    if ((fp = fopen("/proc/mounts", "r")) == NULL) {
        errno = EIO;
        return RETURN_ERR;
    }

    int n_mounts = 0;
    while (!feof(fp)) {
        /* count lines to determine */
        const int ch = fgetc(fp);         /* size of struct array     */
        if (ch == '\n')
            n_mounts++;
    }

    if (n_mounts <= 0) {
        fclose(fp);
        errno = EIO;
        return RETURN_ERR;
    }

    /* If the buffer size is FS_NUM, the user
     * only wants the number of mounts */
    if (!struct_array_buf) {
        fclose(fp);
        return n_mounts;
    }

    rewind(fp);

    /* Get the mount info for each mounted FS. */
    struct mounted_fs_entry mounted_fs_struct[n_mounts];
    char line_buf[256];

    for (int i = 0; i < n_mounts; i++) {
        if (fgets(line_buf, sizeof(line_buf), fp) == NULL) {
            break;
        }

        int parsed_fields = sscanf(line_buf, "%s %s %s %s %d %d\n",
                                   mounted_fs_struct[i].fs_spec, mounted_fs_struct[i].fs_file,
                                   mounted_fs_struct[i].fs_vsftype, mounted_fs_struct[i].fs_mntops,
                                   &mounted_fs_struct[i].fs_freq, &mounted_fs_struct[i].fs_passno);

        if (parsed_fields < 6) {
            fprintf(stderr, "Error parsing line: %s\n", line_buf);
        }
    }
    fclose(fp);

    struct statfs def_struct_tmp = {0};
    struct statfs_ext tmp_buf = {0};
    const size_t n_structs = bufsize / sizeof(struct statfs_ext);
    int filled_structs = 0;

    for (size_t i = 0; i < n_structs; i++) {
        memset(&tmp_buf, 0, sizeof(struct statfs_ext)); // Reset for each loop

        /* statfs directly using the known mount point */
        if (statfs(mounted_fs_struct[i].fs_file, &def_struct_tmp) == 0) {
            merge_statfs_structs(&def_struct_tmp, &tmp_buf);
        } else {
            /* Handled failed statfs (e.g. permission denied on FUSE mounts) */
            continue;
        }

        filled_structs++;
        /* Copy the known good strings from our local struct */
        strncpy(tmp_buf.f_fstypename, mounted_fs_struct[i].fs_vsftype, FS_TYPE_LEN - 1);
        strncpy(tmp_buf.f_mntonname, mounted_fs_struct[i].fs_file, PATH_MAX - 1);
        strncpy(tmp_buf.f_mntfromname, mounted_fs_struct[i].fs_spec, PATH_MAX - 1);

        memcpy(&struct_array_buf[i], &tmp_buf, sizeof(struct statfs_ext));
    }


    /* number of statfs_ext structs we actually filled. */
    return filled_structs;
}

#ifdef __linux__
/* Linux route: use your custom struct and function */
typedef struct statfs_ext vfs_stat_t;

static int vfs_statfs(const char *path, vfs_stat_t *buf) {
    return statfs_ext(path, buf);
}

static int vfs_getfsstat(vfs_stat_t *buf, const size_t bufsize) {
    return getfsstat_ext(buf, bufsize);
}

#else
/* macOS/FreeBSD route: use the native OS struct and syscall */
#define _DARWIN_FEATURE_64_BIT_INODE 1

#include <sys/param.h>
#include <sys/mount.h>

typedef struct statfs vfs_stat_t;

static inline int vfs_statfs(const char *path, vfs_stat_t *buf) {
    return statfs(path, buf);
}

static inline int vfs_getfsstat(vfs_stat_t *buf, const size_t bufsize) {
    return getfsstat(buf, bufsize, MNT_DWAIT);
}
#endif

#endif /* MOUNT_H */
