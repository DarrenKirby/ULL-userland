/***************************************************************************
 *   uptime.c - Tell how long the system has been running                  *
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
#include <stddef.h>
#include <utmpx.h>
#include <time.h>

#ifdef  __linux__
#include <sys/sysinfo.h>
#else
#include <sys/sysctl.h>
#endif

#ifdef __FreeBSD__
#include <sys/types.h>
#include <sys/param.h>
#include <vm/vm_param.h>
#endif

const char *APPNAME = "uptime";

#define ONE_DAY     86400
#define ONE_HOUR    3600
#define ONE_MINUTE  60
#define LOADS_SCALE 65536.0

int get_num_users(void) {
    struct utmpx *utmp_struct;
    int num_user = 0;
    setutxent();
    while ((utmp_struct = getutxent())) {
        if (utmp_struct->ut_type == USER_PROCESS &&
            utmp_struct->ut_user[0] != '\0')
            num_user++;
    }
    endutxent();
    return num_user;
}

/* Print the current time */
void get_time(void) {
    time_t the_time;
    (void) time(&the_time);
    const struct tm *tm_ptr = localtime(&the_time);
    printf(" %02d:%02d:%02d", tm_ptr->tm_hour, tm_ptr->tm_min, tm_ptr->tm_sec);
}

static int print_uptime(void) {
    int error = 0;
    int num_user;
    long int days, hours, minutes;
    long int uptime_in_days, uptime_in_hours, uptime_in_seconds;

    float av1, av2, av3;
#ifdef __linux__
    /* Get uptime Linux */
    struct sysinfo s_info;
    error = sysinfo(&s_info);

    uptime_in_seconds = s_info.uptime;

    /* Get load average Linux */
    av1 = s_info.loads[0] / LOADS_SCALE;
    av2 = s_info.loads[1] / LOADS_SCALE;
    av3 = s_info.loads[2] / LOADS_SCALE;
#else
    /* Get uptime OS X / *BSD */
    struct timeval boot_time;
    size_t len = sizeof(boot_time);
    int mib[2] = { CTL_KERN, KERN_BOOTTIME };

    if( sysctl(mib, 2, &boot_time, &len, NULL, 0) < 0 ) {
        printf("Error getting uptime");
        error = -1;
    }

    const time_t boot_sec = boot_time.tv_sec;
    const time_t current_sec = time(NULL);
    uptime_in_seconds = (long)difftime(current_sec, boot_sec);

    /* Get load average *BSD */
#ifdef __FreeBSD__
    double loadavg[3];
    if (getloadavg(loadavg, nitems(loadavg)) == -1) {
        printf("Error getting load average");
        error = -1;
    }
    av1 = loadavg[0];
    av2 = loadavg[1];
    av3 = loadavg[2];

    /* Get load average OS X */
#else
    struct loadavg loads;
    size_t load_len = sizeof(loads);
    int mib2[2] = { CTL_VM, VM_LOADAVG };

    if (sysctl(mib2, 2, &loads, &load_len, NULL, 0) < 0) {
        printf("Error getting load average");
        error = -1;
    }

    av1 = (float)loads.ldavg[0] / (float)loads.fscale;
    av2 = (float)loads.ldavg[1] / (float)loads.fscale;
    av3 = (float)loads.ldavg[2] / (float)loads.fscale;
#endif
#endif

    /* Factor seconds into larger units */
    days = uptime_in_seconds / ONE_DAY;
    uptime_in_days = uptime_in_seconds - (days * ONE_DAY);
    hours = uptime_in_days / ONE_HOUR;
    uptime_in_hours = uptime_in_days - hours * ONE_HOUR;
    minutes = uptime_in_hours / ONE_MINUTE;

    num_user = get_num_users();

    printf("  up %ld day%s %02ld:%02ld,  %i user%s,  load average: %2.2f, %2.2f, %2.2f\n", days,
        (days != 1) ? "s" : "", hours, minutes, num_user, (num_user != 1) ? "s" : "", av1, av2, av3);

    return error;
}

static void showHelp(void) {
    printf("Usage: %s [-V, --version] [-h, --help]\n\n\
    -V, --version\tdisplay version\n \
    -h, --help\t\tdisplay this help\n\n \
    Report bugs to <bulliver@gmail.com>\n", APPNAME);
}

int main(const int argc, char *argv[]) {
    int opt;

    const struct option long_opts[] = {
        {"help", 0, NULL, 'h'},
        {"version", 0, NULL, 'V'},
        {NULL,0,NULL,0}
    };

    while ((opt = getopt_long(argc, argv, "Vh", long_opts, NULL)) != -1) {
        switch(opt) {
            case 'V':
                printf("%s (%s) version %s\n", APPNAME, APPSUITE, APPVERSION);
                printf("%s compiled on %s at %s\n",
                       strrchr(__FILE__, '/') ? strrchr(__FILE__, '/') + 1 : __FILE__,
                       __DATE__, __TIME__);
                exit(EXIT_SUCCESS);
            case 'h':
                showHelp();
                exit(EXIT_SUCCESS);
            default:
                showHelp();
                exit(EXIT_FAILURE);
        }
    }

    get_time();
    if (print_uptime() == 0)
        return EXIT_SUCCESS;
    return EXIT_FAILURE;
}
