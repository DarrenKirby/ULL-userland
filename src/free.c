/***************************************************************************
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

#include <errno.h>
#include <unistd.h>

#define APPNAME "free"
#include "common.h"

unsigned long int fmt(unsigned long n, int C);

int getfree(int C, int T, int BC) {
    struct meminfo {
        unsigned long int memtotal;
        unsigned long int memfree;
        unsigned long int memavialable;
        unsigned long int buffers;
        unsigned long int cached;
        unsigned long int swapcached;
        unsigned long int active;
        unsigned long int inactive;
        unsigned long int a_active;
        unsigned long int a_inactive;
        unsigned long int f_active;
        unsigned long int f_unactive;
        unsigned long int unevictable;
        unsigned long int mlocked;
        unsigned long int swaptotal;
        unsigned long int swapfree;
        unsigned long int dirty;
        unsigned long int writeback;
        unsigned long int anonpages;
        unsigned long int mapped;
        unsigned long int shmem;
        unsigned long int slab;
        unsigned long int sreclaimable;

    };

    int i;
    struct meminfo minfo;
    char lines[35];
    char *line = lines;
    char memtype[15];
    long int value[23];

    FILE *memi;
    memi = fopen("/proc/meminfo", "r"); /* hideously non-portable */

    if (!memi) {
        printf("open failed\n");
        return EXIT_FAILURE;
    }

    for (i = 0; i < 23; i++) {
        line = fgets(lines, 30, memi);
        sscanf(line, "%s%ld kB", memtype, &value[i]);
    }

    minfo = (struct meminfo) { value[0],  value[1],  value[2],  value[3],
                               value[4],  value[5],  value[6],  value[7],
                               value[8],  value[9],  value[10], value[11],
                               value[12], value[13], value[14], value[15],
                               value[16], value[17], value[18], value[19],
                               value[20], value[21], value[22]};

    long int memused;
    long int used_minus_buffer;
    long int free_plus_buffer;
    int used_swap;

    memused = minfo.memtotal - minfo.memfree;
    used_minus_buffer = memused - (minfo.buffers + minfo.cached);
    free_plus_buffer = minfo.memfree + minfo.buffers + minfo.cached;
    used_swap = minfo.swaptotal - minfo.swapfree;

    printf("%18s %10s %10s %10s %10s %10s\n", "total","used","free","shared","buffers", "cached");
    printf("Mem:%14lu %10lu %10lu %10lu %10lu %10lu\n",
            fmt(minfo.memtotal,C), fmt(memused,C), fmt(minfo.memfree,C),
            fmt(minfo.shmem,C), fmt(minfo.buffers,C), fmt(minfo.cached,C));

    if (BC == 1) {
        printf("-/+ buffers/cache: %10lu %10lu\n", fmt(used_minus_buffer,C), fmt(free_plus_buffer,C));
    }
    printf("Swap: %12lu %10lu %10lu\n", fmt(minfo.swaptotal,C), fmt(used_swap,C), fmt(minfo.swapfree,C));

    if (T == 1) {
        printf("Total: %11lu %10lu %10lu\n", fmt(minfo.memtotal + minfo.swaptotal,C), fmt(memused + used_swap,C),
                                             fmt(minfo.memfree + minfo.swapfree,C));
    }
    return 0;
}

unsigned long int fmt(unsigned long n, int C) {
    if (C == 2) {
        return n / 1024;
    } else if (C == 1) {
        return n * 1024;
    } else {
        return n;
    }
}

int showHelp(void) {
    printf("usage: %s [-b|-k|-m] [-o] [-t] [-s delay] [-V, --version] [-h, --help]\n \
    -b,-k,-m\t   show output in bytes, KB, or MB\n \
    -o\t\t   use old format (no -/+buffers/cache line\n \
    -t\t\t   display total for RAM + swap\n \
    -s\t\t   update every [delay] seconds\n \
    -h, --help\t   display this help\n \
    -V, --version display version information and exit\n\n \
    Report bugs to <bulliver@gmail.com>\n", APPNAME);
	return EXIT_SUCCESS;
}

int main(int argc, char *argv[]) {

    int opt;

    int C;         /* -b, -k, or -m */
    int P = 0;     /* polling? */
    int T = 0;     /* print total? */
    int BC = 1;    /* display -/+ buffer/cache? */

    int poll_interval;

    struct option longopts[] = {
        {"help", 0, NULL, 'h'},
        {"version", 0, NULL, 'V'},
        {0,0,0,0}
    };

    while ((opt = getopt_long(argc, argv, "bkmths:oV", longopts, NULL)) != -1) {
        switch(opt) {
            case 'k':
		C = 0;
		break;
            case 'b':
		C = 1;
		break;
            case 'm':
		C = 2;
		break;
            case 's':
		P = 1;
		poll_interval = atoi(optarg);
		break;
            case 'V':
		printf("%s (%s) version %s\n", APPNAME, APPSUITE, APPVERSION);
		printf("%s compiled on %s at %s\n", basename(__FILE__), __DATE__, __TIME__);
		exit(0);
		break;
            case 't':
		T = 1;
		break;
            case 'o':
		BC = 0;
		break;
            case 'h':
		showHelp();
		return EXIT_SUCCESS;
            default:
		showHelp();
		return EXIT_FAILURE;
        }
    }

    if (P == 1) {
        while (1 == 1) {
            getfree(C,T,BC);
            printf("\n");
            sleep(poll_interval);
        }
    } else {
        getfree(C,T,BC);
        return EXIT_SUCCESS;
    }

}
