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


#define APPNAME "uptime"

#include <sys/sysinfo.h>
#include "common.h"

#if defined(__APPLE__) && defined(__MACH__)
int main() {
    printf("Bug: Not working on OS X (and probably *BSD)\nneed to find substitute for clearenv()");
    return EXIT_SUCCESS;
}

#else



#define ONEDAY  86400
#define ONEHOUR  3600
#define ONEMINUTE  60
#define LOADS_SCALE 65536.0

int error;

static int getTime(void) {
    struct tm *tm_ptr;
    time_t the_time;

    (void) time(&the_time);
    tm_ptr = localtime(&the_time);

    printf(" %02d:%02d:%02d", tm_ptr->tm_hour, tm_ptr->tm_min, tm_ptr->tm_sec);
    return EXIT_SUCCESS;
}

static int getUptime(void) {
    struct sysinfo s_info;
    error = sysinfo(&s_info);

    int days, hours, minutes;
    long int upmind, upminh, uptimes;

    uptimes = s_info.uptime; /* returned in seconds */
    days = uptimes / ONEDAY;
    upmind = uptimes - (days * ONEDAY);
    hours = upmind / ONEHOUR;
    upminh = upmind - hours * ONEHOUR;
    minutes = upminh / ONEMINUTE;

    float av1, av2, av3;
    av1 = s_info.loads[0] / LOADS_SCALE;
    av2 = s_info.loads[1] / LOADS_SCALE;
    av3 = s_info.loads[2] / LOADS_SCALE;

    /* This next block is stolen fron GNU uptime */
    struct utmp *utmpstruct;
    int numuser = 0;
    setutent();
    while ((utmpstruct = getutent())) {
        if ((utmpstruct->ut_type == USER_PROCESS) &&
            (utmpstruct->ut_name[0] != '\0'))
            numuser++;
    }
    endutent();

    printf(" up %i day%s, %02i:%02i, %i user%s, load average: %2.2f, %2.2f, %2.2f\n",
            days, (days != 1) ? "s" : "", hours, minutes, numuser, (numuser != 1) ? "s" : "", av1, av2, av3);
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

    getTime();
    error = getUptime();
    if (error == 0)
        return EXIT_SUCCESS;
    else
        return EXIT_FAILURE;
}
#endif
