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
#include <string.h>
#include <sys/types.h>

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
    __WORD_TYPE  f_type;             /* type of filesystem (see below) */
    __WORD_TYPE  f_bsize;            /* optimal transfer block size */
    fsblkcnt_t   f_blocks;           /* total data blocks in filesystem */
    fsblkcnt_t   f_bfree;            /* free blocks in fs */
    fsblkcnt_t   f_bavail;           /* free blocks available to unprivileged user */
    fsfilcnt_t   f_files;            /* total file nodes in filesystem */
    fsfilcnt_t   f_ffree;            /* free file nodes in fs */
    fsid_t       f_fsid;             /* filesystem id */
    __WORD_TYPE  f_namelen;          /* maximum length of filenames */
    __WORD_TYPE  f_frsize;           /* fragment size (since Linux 2.6) */
    __WORD_TYPE  f_spare[5];
    
    /* these extra fields add path info as in the *BSD versions of statfs() */
    char f_fstypename[FS_TYPE_LEN];  /* fs type name */
    char f_mntonname[PATH_MAX];      /* directory on which mounted */
    char f_mntfromname[PATH_MAX];    /* mounted file sytem */
};

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

int getfsstat_linux(struct statfs_ext *buf, long int bufsize) {
    FILE *fp;
    int  n_lines = 0;
    char lines[128];
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

        printf("f_type:    %#x\n", (unsigned int)sas[i2].f_type);
        printf("f_bsize:   %lu\n", sas[i2].f_bsize);
        printf("f_blocks:  %d\n", (int)sas[i2].f_blocks);
        printf("f_bfree:   %d\n", (int)sas[i2].f_bfree);
        printf("f_bavail:  %d\n", (int)sas[i2].f_bavail);
        printf("f_files:   %d\n", (int)sas[i2].f_files);
        printf("f_ffree:   %d\n", (int)sas[i2].f_ffree);
        //printf("f_fsid1:   %lu\n", f_tmp->f_fsid);
        printf("f_namelen  %lu\n", sas[i2].f_namelen);
        printf("f_frsize   %lu\n", sas[i2].f_frsize);
        printf("f_fstypename:  %s\n", sas[i2].f_fstypename);
        printf("f_mntonname:   %s\n", sas[i2].f_mntonname);
        printf("f_mntfromname: %s\n", sas[i2].f_mntfromname);
    }
    
    free(s_tmp);
    free(f_tmp);
    printf("f_fstypename:  %s\n", sas[0].f_fstypename);
    printf("f_fstypename:  %s\n", sas[1].f_fstypename);
    printf("f_fstypename:  %s\n", sas[2].f_fstypename);
    printf("f_fstypename:  %s\n", sas[3].f_fstypename);
    printf("f_fstypename:  %s\n", sas[4].f_fstypename);
    printf("f_fstypename:  %s\n", sas[5].f_fstypename);
    printf("f_fstypename:  %s\n", sas[6].f_fstypename);
    printf("f_fstypename:  %s\n", sas[7].f_fstypename);
    printf("f_fstypename:  %s\n", sas[8].f_fstypename);
    printf("f_fstypename:  %s\n", sas[9].f_fstypename);
    printf("Size of sas: %lu\n", sizeof(sas));
    printf("Size of buf: %lu\n", sizeof(buf));
    //buf = **sas;
    free(sas);
    return i2;
}

#endif /* _MOUNT_H */


