/***************************************************************************
 *   free.c - report memory usage                                          *
 *                                                                         *
 *   Copyright (C) 2014-2015 by Darren Kirby                               *
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


#define APPNAME "free"
#include "common.h"

struct optstruct {
    int C;         /* -b, -k, or -m */
    char base;     /* for OS X */
    int P;         /* polling? */
    int T;         /* print total? */
    int BC;        /* display -/+ buffer/cache? */
} opts;

#if defined (__linux__)
static int get_free(void) {
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
            fmt(minfo.memtotal), fmt(memused), fmt(minfo.memfree),
            fmt(minfo.shmem), fmt(minfo.buffers), fmt(minfo.cached));

    if (opts.BC == 1) {
        printf("-/+ buffers/cache: %10lu %10lu\n", fmt(used_minus_buffer), fmt(free_plus_buffer));
    }
    printf("Swap: %12lu %10lu %10lu\n", fmt(minfo.swaptotal), fmt(used_swap), fmt(minfo.swapfree));

    if (opts.T == 1) {
        printf("Total: %11lu %10lu %10lu\n", fmt(minfo.memtotal + minfo.swaptotal), fmt(memused + used_swap),
                                             fmt(minfo.memfree + minfo.swapfree));
    }
    return 0;
}

static unsigned long int fmt(unsigned long n) {
    if (opts.C == 2) {
        return n / 1024;
    } else if (opts.C == 1) {
        return n * 1024;
    } else {
        return n;
    }
}

#elif defined (__APPLE__) && defined (__MACH__)
#include <sys/sysctl.h>

struct Meminfo {
    /* core */
    long int mem_total;
    long int mem_used;
    long int mem_free;
    /* swap */
    long int swap_total;
    long int swap_used;
    long int swap_free;
} m_info;

static int get_swap(void) {
    struct xsw_usage vmusage;
    size_t size = sizeof(vmusage);

    if (sysctlbyname("vm.swapusage", &vmusage, &size, NULL, 0) != 0) {
        fprintf(stderr, "Could not collect VM info, errno %d - %s",
                errno, strerror(errno));
        exit(EXIT_FAILURE);
    }


    m_info.swap_total = vmusage.xsu_total;
    m_info.swap_used  = vmusage.xsu_used;
    m_info.swap_free  = vmusage.xsu_avail;

    return 0;
}

static int get_total_mem(void) {
    size_t size;
    long int buf;
    size = sizeof(long int);

    if (sysctlbyname("hw.memsize", &buf, &size, NULL, 0) != 0) {
        fprintf(stderr, "Could not collect VM info, errno %d - %s",
                errno, strerror(errno));
        exit(EXIT_FAILURE);
    }

    m_info.mem_total = buf;
    return 0;
}

static int get_used_mem(void) {
    FILE *fd;

    if ((fd = popen("vm_stat", "r")) == NULL) {
        fprintf(stderr, "popen failed, errno %d - %s",
                errno, strerror(errno));
    }

    char *line = NULL;
    size_t linecap = 0;
    ssize_t linelen;

    /* first line is throwaway */
    if (!(linelen = getline(&line, &linecap, fd)) > 0) {
	perror("linelength");
    }
    long int value[3];

    for (int i = 0; i < 3; i++) {
        linelen = getline(&line, &linecap, fd);
        sscanf(line, "Pages %*s %ld.", &value[i]);
    }

    m_info.mem_used = (value[0] + value[1] + value[2]) * 4096;
    m_info.mem_free = (m_info.mem_total - m_info.mem_used);

    pclose(fd);
    return 0;
}

static long int fmt(long int n) {
    if (opts.base == 'm')
	return n / 1024 / 1024;
    else if (opts.base == 'k')
	return n / 1024;
    else
	return n;
}

static int get_free(void) {
    if (get_total_mem() != 0)
	printf("Could not get total memory\n");
    if (get_used_mem() != 0)
	printf("Could not obtain used memory\n");
    if (get_swap() != 0)
	printf("Could not obtain swap memory\n");
	printf("\t%10s\t%10s\t%10s\n", "Total", "Used", "Free");
	printf("Mem:\t%10ld\t%10ld\t%10ld\n", fmt(m_info.mem_total),fmt(m_info.mem_used),fmt(m_info.mem_free));
	printf("Swap:\t%10ld\t%10ld\t%10ld\n", fmt(m_info.swap_total),fmt(m_info.swap_used),fmt(m_info.swap_free));
    return 0;
}

#endif

static int showHelp(void) {
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
    int poll_interval = 0;

    struct option longopts[] = {
        {"help", 0, NULL, 'h'},
        {"version", 0, NULL, 'V'},
        {0,0,0,0}
    };

    while ((opt = getopt_long(argc, argv, "bkmths:oV", longopts, NULL)) != -1) {
        switch(opt) {
            case 'k':
		opts.C = 0;
		opts.base = 'k';
		break;
            case 'b':
		opts.C = 1;
		opts.base = 'b';
		break;
            case 'm':
		opts.C = 2;
		opts.base = 'm';
		break;
            case 's':
		opts.P = 1;
		poll_interval = atoi(optarg);
		break;
            case 'V':
		printf("%s (%s) version %s\n", APPNAME, APPSUITE, APPVERSION);
		printf("%s compiled on %s at %s\n", basename(__FILE__), __DATE__, __TIME__);
		return EXIT_SUCCESS;
            case 't':
		opts.T = 1;
		break;
            case 'o':
		opts.BC = 0;
		break;
            case 'h':
		showHelp();
		return EXIT_SUCCESS;
            default:
		showHelp();
		return EXIT_FAILURE;
        }
    }

    if (opts.P == 1) {
        while (1 == 1) {
#if defined (__linux__)
            get_free();
#elif defined (__APPLE__) && defined (__MACH__)
	    get_free();
#endif
            printf("\n");
            sleep(poll_interval);
        }
    } else {
#if defined (__linux__)
        get_free();
#elif defined (__APPLE__) && defined (__MACH__)
	get_free();
#endif
        return EXIT_SUCCESS;
    }

}
