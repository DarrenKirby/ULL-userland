/***************************************************************************
 *   uname.c - print system information                                    *
 *                                                                         *
 *   Copyright (C) 2014 - 2025 Darren Kirby                                *
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

#include "common.h"
#include <sys/utsname.h>
#ifndef __linux__
#include <sys/sysctl.h>
#endif

const char *APPNAME = "uname";

struct packed_flags {
    unsigned int s : 1;
    unsigned int n : 1;
    unsigned int r : 1;
    unsigned int v : 1;
    unsigned int m : 1;
    unsigned int p : 1;
    unsigned int i : 1;
    unsigned int o : 1;
};

static void show_help(void) {
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
Report bugs to <bulliver@gmail.com>\n", APPNAME);
}

int main(const int argc, char *argv[]) {
    int opt;

    const struct option long_opts[] = {
        {"all", 0, nullptr, 'a'},
        {"kernel-name", 0, nullptr, 's'},
        {"nodename", 0, nullptr, 'n'},
        {"kernel-release", 0, nullptr, 'r'},
        {"kernel-version", 0, nullptr, 'v'},
        {"machine", 0, nullptr, 'm'},
        {"processor", 0, nullptr, 'p'},
        {"hardware-platform", 0, nullptr, 'i'},
        {"operating-system", 0, nullptr, 'o'},
        {"help", 0, nullptr, 'h'},
        {"version", 0, nullptr, 'V'},
        {nullptr,0,nullptr,0}
    };

    struct packed_flags optflags = { 0,0,0,0,0,0,0,0 };

    while ((opt = getopt_long(argc, argv, "snrvmpioaVh", long_opts, nullptr)) != -1) {
        switch(opt) {
            case 'V':
                printf("%s (%s) version %s\n", APPNAME, APPSUITE, APPVERSION);
                printf("%s compiled on %s at %s\n",
                       strrchr(__FILE__, '/') ? strrchr(__FILE__, '/') + 1 : __FILE__,
                       __DATE__, __TIME__);
                exit(EXIT_SUCCESS);
            case 'h':
                show_help();
                exit(EXIT_SUCCESS);
            case 'a':
                optflags.s = 1;     /* all the below */
                optflags.n = 1;
                optflags.r = 1;
                optflags.v = 1;
                optflags.m = 1;
                optflags.p = 1;
                optflags.i = 1;
                optflags.o = 1;
                break;
            case 's':
                optflags.s = 1;     /* kernel name */
                break;
            case 'n':
                optflags.n = 1;     /* network node hostname */
                break;
            case 'r':
                optflags.r = 1;     /* kernel release */
                break;
            case 'v':
                optflags.v = 1;     /* kernel version */
                break;
            case 'm':
                optflags.m = 1;     /* machine hardware name */
                break;
            case 'p':
                optflags.p = 1;     /* processor type or 'unknown' */
                break;
            case 'i':
                optflags.i = 1;     /* hardware platform or 'unknown' */
                break;
            case 'o':
                optflags.o = 1;     /* operating system */
                break;
            default:
                show_help();
                exit(EXIT_FAILURE);
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
    uname(&uts);

    int t = 0;

    if (optflags.s) {
        printf("%s ", uts.sysname);
        t = 1;
    }
    if (optflags.n) {
        printf("%s ", uts.nodename);
        t = 1;
    }
    if (optflags.r) {
        printf("%s ", uts.release);
        t = 1;
    }
    if (optflags.v) {
        printf("%s ", uts.version);
        t = 1;
    }
    if (optflags.m) {
        printf("%s ", uts.machine);
        t = 1;
    }
    if (optflags.p) {
#if defined (__linux__)
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
    if (optflags.i) {
#if defined (__linux__)
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
    if (optflags.o) {
#if defined (__linux__)
        printf("GNU/Linux");
#elif defined (BSD) && defined (__unix__)
        printf("%s ", uts.sysname);
#elif defined (__APPLE__) && defined (__MACH__)
        printf("OS X ");
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
