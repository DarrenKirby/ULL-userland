//
//  mount.c
//
//  OS X or *BSD only
//  Created by Darren Kirby on 2015-01-20.
//
//

#include <stdio.h>
#include <stdlib.h>
#include <limits.h>

#include <sys/param.h>
#include <sys/mount.h>
#include <sys/ucred.h>


int main(int argc, char *argv[]) {
    int n_mounts;            /* number of currently mounted filesystems */
    struct statfs *mounted_fs = malloc(sizeof(struct statfs));
    if (mounted_fs == NULL) {
        printf("malloc failed\n");
        exit(1);
    }

    n_mounts = getfsstat(mounted_fs, 8096, MNT_NOWAIT);
    if (n_mounts == -1) {
        printf("getfsstat failed");
        exit(1);
    }

    int i;
    for (i = 0; i < n_mounts; i++) {
        printf("%s on %s (%s", mounted_fs[i].f_mntfromname, mounted_fs[i].f_mntonname, mounted_fs[i].f_fstypename);

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

        printf(")\n");

    }

    free(mounted_fs);
    return 0;
}
