/***************************************************************************
 *   uptime.c - Tell how long the system has been running                  *
 *                                                                         *
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

#include "common.h"

const char *APPNAME = "uptime";

#define ONEDAY  86400
#define ONEHOUR  3600
#define ONEMINUTE  60
#define LOADS_SCALE 65536.0

int get_num_users(void) {
    struct utmpx *utmpstruct;
    int numuser = 0;
    setutxent();
    while ((utmpstruct = getutxent())) {
        if ((utmpstruct->ut_type == USER_PROCESS) &&
            (utmpstruct->ut_user[0] != '\0'))
            numuser++;
    }
    endutxent();
    return numuser;
}


void get_time(void) {
    struct tm *tm_ptr;
    time_t the_time;

    (void) time(&the_time);
    tm_ptr = localtime(&the_time);

    printf(" %02d:%02d:%02d", tm_ptr->tm_hour, tm_ptr->tm_min, tm_ptr->tm_sec);
}


static int print_uptime(void) {

    int error = 0;
    int numuser;
    int days, hours, minutes;
    long int upmind, upminh, uptimes;

    float av1, av2, av3;
#ifdef __linux__
    /* Get uptime Linux */
    struct sysinfo s_info;
    error = sysinfo(&s_info);

    uptimes = s_info.uptime;

    /* Get load average Linux */
    av1 = s_info.loads[0] / LOADS_SCALE;
    av2 = s_info.loads[1] / LOADS_SCALE;
    av3 = s_info.loads[2] / LOADS_SCALE;
#else
    /* Get uptime OS X / *BSD */
    struct timeval boottime;
    size_t len = sizeof(boottime);
    int mib[2] = { CTL_KERN, KERN_BOOTTIME };

    if( sysctl(mib, 2, &boottime, &len, NULL, 0) < 0 ) {
        printf("Error getting uptime");
        error = -1;
    }

    time_t bsec = boottime.tv_sec, csec = time(NULL);
    uptimes = difftime(csec, bsec);

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
    size_t lenl = sizeof(loads);
    int mib2[2] = { CTL_VM, VM_LOADAVG };

    if (sysctl(mib2, 2, &loads, &lenl, NULL, 0) < 0) {
        printf("Error getting load average");
        error = -1;
    }

    av1 = loads.ldavg[0] / (float)loads.fscale;
    av2 = loads.ldavg[1] / (float)loads.fscale;
    av3 = loads.ldavg[2] / (float)loads.fscale;
#endif
#endif

    /* uptimes returned in seconds */
    days = uptimes / ONEDAY;
    upmind = uptimes - (days * ONEDAY);
    hours = upmind / ONEHOUR;
    upminh = upmind - hours * ONEHOUR;
    minutes = upminh / ONEMINUTE;

    numuser = get_num_users();

    printf("  up %i day%s %02i:%02i,  %i user%s,  load average: %2.2f, %2.2f, %2.2f\n", days,
        (days != 1) ? "s" : "", hours, minutes, numuser, (numuser != 1) ? "s" : "", av1, av2, av3);

    return error;
}


static void showHelp(void) {
    printf("Usage: %s [-V, --version] [-h, --help]\n\n \
    -V, --version\tdisplay version\n \
    -h, --help\t\tdisplay this help\n\n \
    Report bugs to <bulliver@gmail.com>\n", APPNAME);
}


int main(int argc, char *argv[]) {
    int opt;

    struct option longopts[] = {
        {"help", 0, NULL, 'h'},
        {"version", 0, NULL, 'V'},
        {0,0,0,0}
    };

    while ((opt = getopt_long(argc, argv, "Vh", longopts, NULL)) != -1) {
        switch(opt) {
            case 'V':
                printf("%s (%s) version %s\n", APPNAME, APPSUITE, APPVERSION);
                printf("%s compiled on %s at %s\n", basename(__FILE__), __DATE__, __TIME__);
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
    else
        return EXIT_FAILURE;
}

