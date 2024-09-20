/***************************************************************************
 *   mount.c - display file attributes                                     *
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


#include <stdio.h>
#include <stdlib.h>
#include <sys/param.h>
#include <sys/mount.h>

#ifndef __linux__
#include <sys/ucred.h>
#else
#include "mount.h"
#endif // __linux__


const char *APPNAME = "mount";

int main(int argc, char *argv[]) {
    int n_mounts; /* number of currently mounted filesystems */
#ifdef __linux__
    struct statfs_ext *mounted_fs = malloc(sizeof(struct statfs_ext));
#else
    struct statfs *mounted_fs = malloc(sizeof(struct statfs));
#endif // __linux__
    if (mounted_fs == NULL) {
        printf("malloc failed\n");
        exit(1);
    }

#ifdef __linux__
    n_mounts = getfsstat_linux(mounted_fs);
#else
    n_mounts = getfsstat(NULL, 0, MNT_NOWAIT);
    n_mounts = getfsstat(mounted_fs, sizeof(struct statfs)*n_mounts, MNT_NOWAIT);
#endif // __linux__
    if (n_mounts == -1) {
        printf("getfsstat failed");
        exit(1);
    }

    int i;
    for (i = 0; i < n_mounts; i++) {
        printf("%s on %s (%s", mounted_fs[i].f_mntfromname, mounted_fs[i].f_mntonname, mounted_fs[i].f_fstypename);

#ifndef __linux__
        // print out FS flags - still more to add
        if ((mounted_fs[i].f_flags & MNT_AUTOMOUNTED) != 0)
            printf(", automounted");
        if ((mounted_fs[i].f_flags & MNT_DONTBROWSE) != 0)
            printf(", nobrowse");
        if ((mounted_fs[i].f_flags & MNT_RDONLY) != 0)
            printf(", read-only");
        if ((mounted_fs[i].f_flags & MNT_JOURNALED) != 0)
            printf(", journaled");
        if ((mounted_fs[i].f_flags & MNT_LOCAL) != 0)
            printf(", local");
        if ((mounted_fs[i].f_flags & MNT_NOSUID) != 0)
            printf(", nosuid");
        if ((mounted_fs[i].f_flags & MNT_QUOTA) != 0)
            printf(", quota");
        if ((mounted_fs[i].f_flags & MNT_ROOTFS) != 0)
            printf(", root");
#endif
        printf(")\n");

    }

    free(mounted_fs);
    return 0;
}
