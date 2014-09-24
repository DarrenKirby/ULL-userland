/***************************************************************************
 *   mount.h - implements the BSD* syscall getfsstat() for Linux           *
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

#ifndef _MOUNT_H
#define _MOUNT_H

#include <stdio.h>
#include <sys/statfs.h>      /* for statfs struct */
#include <linux/limits.h>    /* for PATH_MAX */
#include <errno.h>           /* for perror() */
#include <stdlib.h>          /* for EXIT_FAILURE */

/* prototype */
int getfsstat_linux(struct statfs *buf, long int bufsize);


typedef struct _mounted_fs_entry {
    char fs_spec[PATH_MAX];   /* device or special file system path */
    char fs_file[PATH_MAX];   /* mount point */
    char fs_vsftype[1024];    /* file system type */
    char fs_mntops[1024];     /* mount flags */
    int  fs_freq;             /* dump */
    int  fs_passno;           /* pass */
} mounted_fs_entry;


int getfsstat_linux(struct statfs *buf, long int bufsize) {
    FILE *fp;
    int  n_lines = 0;
    char lines[128];
    char *line = lines;
    
    if ((fp = fopen("/etc/mtab", "r")) == NULL) {
        perror("could not open /etc/mtab");
        exit(EXIT_FAILURE);
    }
    
    char ch;
    while(!feof(fp)) {             /* count lines to determine */
        ch = fgetc(fp);            /* size of struct array     */
        if(ch == '\n') 
            n_lines++;
    }
    rewind(fp);
    
    mounted_fs_entry fse[n_lines];
    int i, i2;

    for (i = 0; i < n_lines; i++) {
        line = fgets(lines, 512, fp);
        sscanf(line, "%s %s %s %s %d %d\n", fse[i].fs_spec, fse[i].fs_file,
                                            fse[i].fs_vsftype, fse[i].fs_mntops,
                                            &fse[i].fs_freq, &fse[i].fs_passno);
        /*) != 0) {
            perror("scanf failed");
            exit(EXIT_FAILURE);
        } */
    //i++;
    }

    fclose(fp);
    struct statfs *baz; //[n_lines];
    
    if (buf == NULL)
        return n_lines;
    
    for (i2 = 0; i2 < n_lines; i2++) {
        statfs(fse[i2].fs_file, &baz); //[i2]);
        printf("filepath: %s\n", fse[i2].fs_file);
        //buf[i2].f_type = baz->f_type;
        //buf[i2].f_bsize = baz->f_bsize;
        //buf[i2].f_blocks = baz->f_blocks;
        
    }
    return i2;
}

#endif /* _MOUNT_H */