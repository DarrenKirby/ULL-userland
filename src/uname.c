/***************************************************************************
 *   uname.c - print system information                                    *
 *                                                                         *
 *   Copyright (C) 2014 - 2026 Darren Kirby                                *
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
#include <sys/utsname.h>
#ifndef __linux__
#include <sys/sysctl.h>
#endif

#include "common.h"


static const char *APP_NAME = "uname";

static struct {
    bool s :1;
    bool n :1;
    bool r :1;
    bool v :1;
    bool m :1;
    bool p :1;
    bool i :1;
    bool o :1;
} opts = {
    .s = true,
    .n = false,
    .r = false,
    .v = false,
    .m = false,
    .p = false,
    .i = false,
    .o = false };

static void show_help()
{
    printf("Usage: %s [OPTION]...\n\n\
    Print certain system information. With no OPTION, same as -s.\n\n\
Options:\n\
    -a, --all\t\t\t print all information, in the following order:\n\
    -s, --kernel-name\t\t print the kernel name\n\
    -n, --nodename\t\t print the network node hostname\n\
    -r, --kernel-release\t print the kernel release\n\
    -v, --kernel-version\t print the kernel version\n\
    -m, --machine\t\t print the machine hardware name\n\
    -p, --processor\t\t print the processor type or 'unknown'\n\
    -i, --hardware-platform\t print the hardware platform or 'unknown'\n\
    -o, --operating-system\t print the operating system\n\n\
    -h, --help\t\t display this help\n\
    -V, --version\t display version information\n\n\
Report bugs to <darren@dragonbyte.ca>\n", APP_NAME);
}

int main(const int argc, char *argv[])
{
    const struct option long_opts[] = {
        {.name = "all",               .has_arg = 0, .flag = nullptr, .val = 'a'},
        {.name = "kernel-name",       .has_arg = 0, .flag = nullptr, .val = 's'},
        {.name = "nodename",          .has_arg = 0, .flag = nullptr, .val = 'n'},
        {.name = "kernel-release",    .has_arg = 0, .flag = nullptr, .val = 'r'},
        {.name = "kernel-version",    .has_arg = 0, .flag = nullptr, .val = 'v'},
        {.name = "machine",           .has_arg = 0, .flag = nullptr, .val = 'm'},
        {.name = "processor",         .has_arg = 0, .flag = nullptr, .val = 'p'},
        {.name = "hardware-platform", .has_arg = 0, .flag = nullptr, .val = 'i'},
        {.name = "operating-system",  .has_arg = 0, .flag = nullptr, .val = 'o'},
        {.name = "help",              .has_arg = 0, .flag = nullptr, .val = 'h'},
        {.name = "version",           .has_arg = 0, .flag = nullptr, .val = 'V'},
        {.name = nullptr,             .has_arg = 0, .flag = nullptr, .val = 0}
    };
    
    int opt;
    while ((opt = getopt_long(argc, argv, "snrvmpioaVh", long_opts, nullptr)) != -1) {
        switch(opt) {
            case 'V':
                printf("%s (%s) version %s\n", APP_NAME, APP_SUITE, APP_VERSION);
                printf("%s compiled on %s at %s\n",
                       strrchr(__FILE__, '/') ? strrchr(__FILE__, '/') + 1 : __FILE__,
                       __DATE__, __TIME__);
                return EXIT_SUCCESS;
            case 'h':
                show_help();
                return EXIT_SUCCESS;
            case 'a':
                /* all the below */
                opts.n = true;
                opts.r = true;
                opts.v = true;
                opts.m = true;
                opts.p = true;
                opts.i = true;
                opts.o = true;
                break;
            case 's':
                opts.s = true;     /* kernel name */
                break;
            case 'n':
                opts.s = false;
                opts.n = true;     /* network node hostname */
                break;
            case 'r':
                opts.s = false;
                opts.r = true;     /* kernel release */
                break;
            case 'v':
                opts.s = false;
                opts.v = true;     /* kernel version */
                break;
            case 'm':
                opts.s = false;
                opts.m = true;     /* machine hardware name */
                break;
            case 'p':
                opts.s = false;
                opts.p = true;     /* processor type or 'unknown' */
                break;
            case 'i':
                opts.s = false;
                opts.i = true;     /* hardware platform or 'unknown' */
                break;
            case 'o':
                opts.s = false;
                opts.o = true;     /* operating system */
                break;
            default:
                show_help();
                return EXIT_FAILURE;
        }
    }

    struct cpu_info {
        char vendor[50];
        char name[50];
    };

#if defined (__linux__)
    struct cpu_info cpu;

    char *line = NULL;
    size_t len = 0;

    FILE *fp;
    fp = fopen("/proc/cpuinfo", "r");  /* Not portable */

    if (!fp) {
        strcpy(cpu.vendor, "Unknown\0");
        strcpy(cpu.name, "Unknown\0");
    } else {
        const char delimiters[] = ":\n";

        if (getline(&line, &len, fp) == -1) {    /* line 1 throwaway*/
            perror("getline");
            exit(EXIT_FAILURE);
        }
        if (getline(&line, &len, fp) == -1) {    /* line 2 throwaway*/
            perror("getline");
            exit(EXIT_FAILURE);
        }

        char *token;
        token = strtok(line, delimiters);
        token = strtok (NULL, delimiters);
        token = trim_whitespace(token);
        strcpy(cpu.vendor, token);

        if (getline(&line, &len, fp) == -1) {    /* line 3 throwaway*/
            perror("getline");
            exit(EXIT_FAILURE);
        }
        if (getline(&line, &len, fp) == -1) {    /* line 4 throwaway*/
            perror("getline");
            exit(EXIT_FAILURE);
        }
        if (getline(&line, &len, fp) == -1) {    /* line 5 throwaway*/
            perror("getline");
            exit(EXIT_FAILURE);
        }

        token = strtok(line, delimiters);
        token = strtok (NULL, delimiters);
        token = trim_whitespace(token);
        strcpy(cpu.name, token);

    }

    fclose(fp);
#endif

    struct utsname uts;
    if (uname(&uts) == -1) {
        fprintf(stderr, "%s: uname() failed: %s\n", APP_NAME, strerror(errno));
        return EXIT_FAILURE;
    }

    int t = 0;

    if (opts.s) {
        printf("%s ", uts.sysname);
        t = 1;
    }
    if (opts.n) {
        printf("%s ", uts.nodename);
        t = 1;
    }
    if (opts.r) {
        printf("%s ", uts.release);
        t = 1;
    }
    if (opts.v) {
        printf("%s ", uts.version);
        t = 1;
    }
    if (opts.m) {
        printf("%s ", uts.machine);
        t = 1;
    }
    if (opts.p) {
#ifdef __linux__
        printf("%s ", cpu.name);
#elif defined (BSD) && defined (__unix__)
        size_t len;
        static char buf[1024];
        if (sysctlbyname("hw.model", &buf, &len, NULL, 0) == -1)
            g_error("could not read sysctl");
        printf("%s ", buf);
#elif defined (__APPLE__) && defined (__MACH__)
        ;
#elif defined (__sun) && defined (__SVR4)
        ;
#endif
        t = 1;
    }
    if (opts.i) {
#ifdef __linux__
        printf("%s ", cpu.vendor);
#elif defined (BSD) && defined (__unix__)
        ;
#elif defined (__APPLE__) && defined (__MACH__)
        ;
#elif defined (__sun) && defined (__SVR4)
        ;
        t = 1;
#endif
    }
    if (opts.o) {
#ifdef __linux__
        printf("GNU/Linux");
#elif defined (BSD) && defined (__unix__)
        printf("%s ", uts.sysname);
#elif defined (__APPLE__) && defined (__MACH__)
        printf("macOS ");
#elif defined (__sun) && defined (__SVR4)
        printf("Solaris");
#endif
        t = 1;
    }
    if (t == 0) {
        printf("%s ", uts.sysname);
    }
    printf("\n");

    return EXIT_SUCCESS;
}
