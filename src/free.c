/***************************************************************************
 *   free.c - report memory usage                                          *
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

#include <getopt.h>
#include <stdint.h>

#include "common.h"


static const char *APP_NAME = "free";

static struct {
    char base;     /* -b, -k, or -m */
    bool poll;     /* polling? */
    bool total;    /* print total? */
    bool bc;       /* display -/+ buffer/cache? */
} opts = {
    .poll = false,
    .total = false,
    .bc = true };

static uint64_t fmt(const uint64_t n)
{
#ifdef __linux__
    if (opts.base == 'b') {
        return n * 1024;
    } else if (opts.base == 'm') {
        return n / 1024;
    } else {
        return n;
    }
#else
    if (opts.base == 'm') {
        return n / 1024 / 1024;
    }
    if (opts.base == 'k') {
        return n / 1024;
    }
    return n;
#endif
}

#ifdef __linux__
static int get_free(void) {
    struct meminfo {
        uint64_t memtotal;
        uint64_t memfree;
        uint64_t memavialable;
        uint64_t buffers;
        uint64_t cached;
        uint64_t swapcached;
        uint64_t active;
        uint64_t inactive;
        uint64_t a_active;
        uint64_t a_inactive;
        uint64_t f_active;
        uint64_t f_unactive;
        uint64_t unevictable;
        uint64_t mlocked;
        uint64_t swaptotal;
        uint64_t swapfree;
        uint64_t dirty;
        uint64_t writeback;
        uint64_t anonpages;
        uint64_t mapped;
        uint64_t shmem;
        uint64_t slab;
        uint64_t sreclaimable;
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
        fprintf(stderr, "%s: failed to open /proc/meminfo: %s\n", APP_NAME, strerror(errno));
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

    uint64_t memused;
    uint64_t used_minus_buffer;
    uint64_t free_plus_buffer;
    uint64_t used_swap;

    memused = minfo.memtotal - minfo.memfree;
    used_minus_buffer = memused - (minfo.buffers + minfo.cached);
    free_plus_buffer = minfo.memfree + minfo.buffers + minfo.cached;
    used_swap = minfo.swaptotal - minfo.swapfree;

    printf("%18s %10s %10s %10s %10s %10s\n", "total","used","free","shared","buffers", "cached");
    printf("Mem:%14lu %10lu %10lu %10lu %10lu %10lu\n",
            fmt(minfo.memtotal), fmt(memused), fmt(minfo.memfree),
            fmt(minfo.shmem), fmt(minfo.buffers), fmt(minfo.cached));

    if (opts.bc == 1) {
        printf("-/+ buffers/cache: %10lu %10lu\n", fmt(used_minus_buffer), fmt(free_plus_buffer));
    }
    printf("Swap: %12lu %10lu %10lu\n", fmt(minfo.swaptotal), fmt(used_swap), fmt(minfo.swapfree));

    if (opts.total == 1) {
        printf("Total: %11lu %10lu %10lu\n", fmt(minfo.memtotal + minfo.swaptotal), fmt(memused + used_swap),
                                             fmt(minfo.memfree + minfo.swapfree));
    }
    return EXIT_SUCCESS;
}

#elif defined (__APPLE__) && defined (__MACH__) || defined(__FreeBSD__)
#include <sys/sysctl.h>

#ifdef __FreeBSD__
#include <vm/vm_param.h>
#include <fcntl.h>
#endif

static struct {
    /* core */
    uint64_t mem_total;
    uint64_t mem_used;
    uint64_t mem_free;
    uint64_t mem_used_bc; /* +/- buffers/cache */
    uint64_t mem_free_bc; /*     ibid          */
    /* swap */
    uint64_t swap_total;
    uint64_t swap_used;
    uint64_t swap_free;
} m_info;

static void get_mem()
{
#ifdef __FreeBSD__
    /* kvm requires privileged access -
     * code below parses output of swapinfo
    struct kvm_swap swapinfo;
    kvm_t *kd;

    kd = kvm_openfiles(NULL, NULL, NULL, O_RDONLY, NULL);
    if (kd == NULL) {
        perror("kvm_openfiles");
        return 1;
    }

    int n = kvm_getswapinfo(kd, &swapinfo, 1, 0);
    if (n < 0) {
        perror("kvm_getswapinfo");
        kvm_close(kd);
        return 1;
    }

    kvm_close(kd);

    long page_size = getpagesize();

    m_info.swap_total = (unsigned long)swapinfo.ksw_total * page_size;
    m_info.swap_used  = (unsigned long)swapinfo.ksw_used  * page_size;
    m_info.swap_used  = m_info.swap_total - m_info.swap_used;
    */

    /* Get core memory stats */
    size_t len;
    long pagesize = getpagesize();
    unsigned long physmem, freepages, inactivepages, cachepages, buffers;

    /* Total physical memory (in bytes) */
    len = sizeof(physmem);
    if (sysctlbyname("hw.physmem", &physmem, &len, NULL, 0) != 0) {
        perror("sysctl hw.physmem");
        return 1;
    }

    /* Free and cache memory (in pages) */
    if (sysctlbyname("vm.stats.vm.v_free_count", &freepages, &len, NULL, 0) != 0 ||
        sysctlbyname("vm.stats.vm.v_inactive_count", &inactivepages, &len, NULL, 0) != 0 ||
        sysctlbyname("vm.stats.vm.v_cache_count", &cachepages, &len, NULL, 0) != 0 ||
        sysctlbyname("vfs.bufspace", &buffers, &len, NULL, 0)) {
        perror("sysctl free/inactive/cache");
    return 1;
    }

    unsigned long free = (freepages + inactivepages + cachepages) / pagesize;
    unsigned long used = physmem - free;
    m_info.mem_used_bc = used - ((cachepages + inactivepages + buffers) / pagesize);
    m_info.mem_free_bc = free + ((cachepages + inactivepages + buffers) / pagesize);
    m_info.mem_total = physmem;
    m_info.mem_used = used;
    m_info.mem_free = free;

    /* Parse swapinfo */
    FILE *fp;
    char buffer[256];
    fp = popen("swapinfo -k", "r");
    if (fp == NULL) {
        perror("popen swapinfo");
    }

    /* Skip header */
    fgets(buffer, sizeof(buffer), fp);

    unsigned long total_kb = 0, used_kb = 0, free_kb = 0;
    while (fgets(buffer, sizeof(buffer), fp) != NULL) {
        char device[64];
        unsigned long t, u, f;
        if (sscanf(buffer, "%s %lu %lu %lu", device, &t, &u, &f) == 4) {
            total_kb += t;
            used_kb += u;
            free_kb += f;
        }
    }
    pclose(fp);
    /* swapinfo figures in 1000k blocks */
    m_info.swap_total = total_kb * 1024;
    m_info.swap_used = used_kb * 1024;
    m_info.swap_free = free_kb * 1024;

#else
    struct xsw_usage vm_usage;
    size_t v_size = sizeof(vm_usage);

    if (sysctlbyname("vm.swapusage", &vm_usage, &v_size, NULL, 0) != 0) {
        fprintf(stderr, "Could not collect VM info, errno %d - %s",
                errno, strerror(errno));
        exit(EXIT_FAILURE);
    }

    m_info.swap_total = vm_usage.xsu_total;
    m_info.swap_used  = vm_usage.xsu_used;
    m_info.swap_free  = vm_usage.xsu_avail;

    size_t i_size;
    long int buf;
    i_size = sizeof(long int);

    if (sysctlbyname("hw.memsize", &buf, &i_size, NULL, 0) != 0) {
        fprintf(stderr, "Could not collect VM info, errno %d - %s\n",
                errno, strerror(errno));
        exit(EXIT_FAILURE);
    }

    m_info.mem_total = buf;

    FILE *fd;

    if ((fd = popen("vm_stat", "r")) == NULL) {
        fprintf(stderr, "popen failed, errno %d - %s",
                errno, strerror(errno));
        exit(EXIT_FAILURE);
    }

    char *line = NULL;
    size_t line_cap = 0;

    /* first line is throwaway */
    if (getline(&line, &line_cap, fd) < 0) {
        perror("getline");
        exit(EXIT_FAILURE);
    }
    long int value[3];

    for (int i = 0; i < 3; i++) {
        getline(&line, &line_cap, fd);
        sscanf(line, "Pages %*s %ld.", &value[i]);
    }

    m_info.mem_used = (value[0] + value[1] + value[2]) * 4096;
    m_info.mem_free = m_info.mem_total - m_info.mem_used;

    pclose(fd);
#endif
}

static int get_free(void)
{
    get_mem();
    printf("\t%10s\t%10s\t%10s\n", "Total", "Used", "Free");
    printf("Mem:\t%10llu\t%10llu\t%10llu\n", fmt(m_info.mem_total),fmt(m_info.mem_used),fmt(m_info.mem_free));
#ifdef __FreeBSD__
    if (opts.BC == 1) {
        printf("+/- buffers/cache:\t%10llu\t   %llu\n", fmt(m_info.mem_used_bc), fmt(m_info.mem_free_bc));
    }
#endif
    printf("Swap:\t%10llu\t%10llu\t%10llu\n", fmt(m_info.swap_total),fmt(m_info.swap_used),fmt(m_info.swap_free));
    return 0;
}
#endif

static void showHelp()
{
    printf("usage: %s [-b|-k|-m] [-o] [-t] [-s delay] [-V, --version] [-h, --help]\n\
Report memory usage\n\n\
Options:\n\
    -b,-k,-m\t   show output in bytes, KB, or MB\n\
    -o\t\t   use old format (no -/+buffers/cache line\n\
    -t\t\t   display total for RAM + swap\n\
    -s\t\t   update every [delay] seconds\n\
    -h, --help\t   display this help\n\
    -V, --version  display version information and exit\n\n\
Report bugs to <darren@dragonbyte.ca>\n", APP_NAME);
}

int main(const int argc, char *argv[])
{
#ifdef __linux__
    opts.base = 'k'; /* Linux shows values in k by default */
#else
    opts.base = 'b';
#endif
    int poll_interval = 0;

    static const struct option long_opts[] = {
        { .name = "help",    .has_arg = no_argument, .flag = nullptr, .val = 'h' },
        { .name = "version", .has_arg = no_argument, .flag = nullptr, .val = 'V' },
        { .name = nullptr,   .has_arg = no_argument, .flag = nullptr, .val = 0 }
    };
    
    /* Min and max vals for parse_numeric_arg. */
    const int *min = &(int){1};
    const int *max = &(int){INT32_MAX};

    int opt;
    /* show +/- buffers/cache by default */
    opts.bc = 1;
    while ((opt = getopt_long(argc, argv, "bkmths:oV", long_opts, NULL)) != -1) {
        switch(opt) {
        case 'k':
            opts.base = 'k';
            break;
        case 'b':
            opts.base = 'b';
            break;
        case 'm':
            opts.base = 'm';
            break;
        case 's':
            opts.poll = 1;
            poll_interval = parse_numeric_arg(optarg, min, max, APP_NAME);
            break;
        case 'V':
            printf("%s (%s) version %s\n", APP_NAME, APP_SUITE, APP_VERSION);
            printf("%s compiled on %s at %s\n",
                   strrchr(__FILE__, '/') ? strrchr(__FILE__, '/') + 1 : __FILE__,
                   __DATE__, __TIME__);
            return EXIT_SUCCESS;
        case 't':
            opts.total = true;
            break;
        case 'o':
            opts.bc = false;
            break;
        case 'h':
            showHelp();
            return EXIT_SUCCESS;
        default:
            showHelp();
            return EXIT_FAILURE;
        }
    }

    /* Polling */
    if (opts.poll) {
        while (1) {
            get_free();
            printf("\n");
            sleep(poll_interval);
        }
    }
    /* Print and quit */
    get_free();
    return EXIT_SUCCESS;
}
