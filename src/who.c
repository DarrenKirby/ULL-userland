/***************************************************************************
 *   who - show logged-in users                                            *
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

#include <unistd.h>

#include <stdlib.h>
#include <utmpx.h>
#include <time.h>

#if defined(__APPLE__) && defined(__MACH__)
#include <sys/sysctl.h>
#endif
#ifdef __linux__
#include <sys/sysinfo.h>
#endif // __linux__

#include "common.h"
const char *APPNAME = "who";

struct optstruct {
    unsigned int quick;
} opts;


static void show_help(void) {
    printf("Usage: %s [OPTION]...\n\n\
Show all logged in users\n \
Options:\n\
    -b, --boot\t\tprint system boot time\n\
    -a, --all\t\tprint boot time and users\n\
    -h, --help\t\tdisplay this help\n\
    -V, --version\tdisplay version information\n\n\
Report bugs to <bulliver@gmail.com>\n", APPNAME);
}


void print_boot_time(void) {
    struct tm *tm_ptr;
#ifdef __linux__
    struct sysinfo s_info;
    int error = sysinfo(&s_info);
    if(error != 0) {
        gen_error("Failed to get uptime\n");
    }
    time_t boot_secs = s_info.uptime;
    time_t current_time = time(NULL);
    time_t boot_time = current_time - boot_secs;

    tm_ptr = localtime(&boot_time);

    char buffer[20];
    strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M", tm_ptr);

    printf("         system boot  %s\n", buffer);
#else
    struct timeval boottime;

    size_t len = sizeof(boottime);
    int mib[2] = { CTL_KERN, KERN_BOOTTIME };
    if( sysctl(mib, 2, &boottime, &len, NULL, 0) < 0 ) {
        printf("Error getting uptime");
    }

    time_t bsec = boottime.tv_sec;

    tm_ptr = localtime(&bsec);

    printf("         system boot    %i-%02d-%02d %02d:%02d:%02d\n", (1900 + tm_ptr->tm_year),
           (1 + tm_ptr->tm_mon), tm_ptr->tm_mday, tm_ptr->tm_hour,
           tm_ptr->tm_min, tm_ptr->tm_sec);
#endif // __linux__
}


void print_users(void) {
    struct utmpx *utmpstruct;
    time_t the_time;
    struct tm *tm_ptr;

    setutxent();
    while ((utmpstruct = getutxent())) {
        if ((utmpstruct->ut_type == USER_PROCESS) &&
            (utmpstruct->ut_user[0] != '\0')) {
                the_time = utmpstruct->ut_tv.tv_sec;
                tm_ptr = localtime(&the_time);
                printf("%s  %s\t%i-%02d-%02d %02d:%02d:%02d\n", utmpstruct->ut_user,
                       utmpstruct->ut_line, (1900 + tm_ptr->tm_year),
                       (1 + tm_ptr->tm_mon), tm_ptr->tm_mday, tm_ptr->tm_hour,
                        tm_ptr->tm_min, tm_ptr->tm_sec);
            }

    }
    endutxent();
}


int main(int argc, char *argv[]) {
    int opt;

    struct option longopts[] = {
        {"help", 0, NULL, 'h'},
        {"version", 0, NULL, 'V'},
        {"all", 0, NULL, 'a'},
        {"boot", 0, NULL, 'b'},
        {"quick", 0, NULL, 'q'},
        {0,0,0,0}
    };

    while ((opt = getopt_long(argc, argv, "Vhbaq", longopts, NULL)) != -1) {
        switch(opt) {
            case 'V':
                printf("%s (%s) version %s\n", APPNAME, APPSUITE, APPVERSION);
                printf("%s compiled on %s at %s\n", basename(__FILE__), __DATE__, __TIME__);
                exit(EXIT_SUCCESS);
                break;
            case 'h':
                show_help();
                exit(EXIT_SUCCESS);
                break;
            case 'b':
                print_boot_time();
                exit(EXIT_SUCCESS);
                break;
            case 'a':
                print_boot_time();
                break;
            case 'q':
                opts.quick = 1;
                break;
            case ':':
                 /* getopt_long prints own error message */
                exit(EXIT_FAILURE);
                break;
            case '?':
                 /* getopt_long prints own error message */
                exit(EXIT_FAILURE);
            default:
                show_help();
                exit(EXIT_FAILURE);
                break;
        }
    }

    print_users();

    return EXIT_SUCCESS;
}
