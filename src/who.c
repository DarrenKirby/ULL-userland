/***************************************************************************
 *   who - show logged-in users                                            *
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

#include <getopt.h>
#include <utmpx.h>
#include <time.h>

#include "common.h"

#if defined(__APPLE__) && defined(__MACH__) || defined(__FreeBSD__)
#include <sys/sysctl.h>
#endif
#ifdef __linux__
#include <sys/sysinfo.h>
#endif // __linux__


static const char *APP_NAME = "who";

static void show_help()
{
    printf("Usage: %s [OPTION]...\n\n\
Show all logged in users\n\
Options:\n\
    -b, --boot\t\t print system boot time\n\
    -a, --all\t\t print boot time and users\n\
    -h, --help\t\t display this help\n\
    -V, --version\t display version information\n\n\
Report bugs to <darren@dragonbyte.ca>\n", APP_NAME);
}

static void print_boot_time()
{
    struct tm *tm_ptr;
#ifdef __linux__
    struct sysinfo s_info;
    int error = sysinfo(&s_info);
    if(error != 0) {
        fprintf(stderr, "%s: sysinfo() failed: %s\n", APP_NAME, strerror(errno));
        exit(EXIT_FAILURE)
    }
    time_t boot_secs = s_info.uptime;
    time_t current_time = time(NULL);
    time_t boot_time = current_time - boot_secs;

    tm_ptr = localtime(&boot_time);

    char buffer[20];
    strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M", tm_ptr);

    printf("         system boot  %s\n", buffer);
#else
    struct timeval boot_time;

    size_t len = sizeof(boot_time);
    int mib[2] = { CTL_KERN, KERN_BOOTTIME };
    if (sysctl(mib, 2, &boot_time, &len, NULL, 0) < 0 ) {
        printf("%s: sysctl() failed: %s", APP_NAME, strerror(errno));
        exit (EXIT_FAILURE);
    }

    const time_t boot_sec = boot_time.tv_sec;

    tm_ptr = localtime(&boot_sec);

    printf("         system boot    %i-%02d-%02d %02d:%02d:%02d\n", 1900 + tm_ptr->tm_year,
           1 + tm_ptr->tm_mon, tm_ptr->tm_mday, tm_ptr->tm_hour,
           tm_ptr->tm_min, tm_ptr->tm_sec);
#endif // __linux__
}


static void print_users()
{
    struct utmpx *utmp_struct;
    time_t the_time;

    setutxent();
    while ((utmp_struct = getutxent())) {
        if (utmp_struct->ut_type == USER_PROCESS &&
            utmp_struct->ut_user[0] != '\0') {
                the_time = utmp_struct->ut_tv.tv_sec;
                const struct tm *tm_ptr = localtime(&the_time);
                printf("%s  %s\t%i-%02d-%02d %02d:%02d:%02d\n", utmp_struct->ut_user,
                       utmp_struct->ut_line, 1900 + tm_ptr->tm_year,
                       1 + tm_ptr->tm_mon, tm_ptr->tm_mday, tm_ptr->tm_hour,
                       tm_ptr->tm_min, tm_ptr->tm_sec);
            }
    }
    endutxent();
}

int main(const int argc, char *argv[])
{
    const struct option long_opts[] = {
        { .name = "help",    .has_arg = no_argument, .flag = nullptr, .val = 'h' },
        { .name = "version", .has_arg = no_argument, .flag = nullptr, .val = 'V' },
        { .name = "all",     .has_arg = no_argument, .flag = nullptr, .val = 'a' },
        { .name = "boot",    .has_arg = no_argument, .flag = nullptr, .val = 'b' },
        { .name = "quick",   .has_arg = no_argument, .flag = nullptr, .val = 'q' },
        { .name = nullptr,   .has_arg = no_argument, .flag = nullptr, .val = 0 }
    };

    int opt;
    while ((opt = getopt_long(argc, argv, "Vhbaq", long_opts, nullptr)) != -1) {
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
            case 'b':
                print_boot_time();
                return EXIT_SUCCESS;
            case 'a':
                print_boot_time();
                break;
            default:
                show_help();
                return EXIT_FAILURE;
        }
    }

    print_users();
    return EXIT_SUCCESS;
}
