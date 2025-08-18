/***************************************************************************
 *   cal - display calendar for a given year/month                         *
 *                                                                         *
 *   Copyright (C) 2014 - 2025 by Darren Kirby                             *
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
#include <time.h>

const char *APPNAME =  "cal";

static void show_help(void) {
    printf("Usage: %s [year] [month]\n\n\
Options:\n\
    -h, --help\t\tdisplay this help\n\
    -V, --version\tdisplay version information\n\n\
Report bugs to <bulliver@gmail.com>\n", APPNAME);
}

/* Given an arbitrary date, return the day of week */
int weekday_sakamoto(int y, const int m, const int d) {
    static int t[] = {0,3,2,5,0,3,5,1,4,6,2,4};
    y -= m < 3;
    return (y + y/4 - y/100 + y/400 + t[m-1] + d) % 7;
}

/* Determine if given year is leap or common */
int is_leap(const int y) {
    return (y % 4 == 0) && ((y % 100 != 0) || (y % 400 == 0));
}

/* Map from month to number of days in month*/
int month_len[2][24] = {
    {31,28,31,30,31,30,31,31,30,31,30,31}, /* common */
    {31,29,31,30,31,30,31,31,30,31,30,31}  /* leap */
};

/* map from month number to month name */
static const char *month_names[12] = {
    "January", "February", "March", "April", "May", "June",
    "July", "August", "September", "October", "November", "December"
};

/* Build a month array
 * args are:
 *       year: the year of the calendar to build
 *          m: the month of the calendar to build
 *   start_wd: the day of the week of the 1st of the month
 *       days: the number of days in the month
 *        buf: the buffer to build the array in
 * just_month: an int flag which tells if we are just printing a single month */
void build_month(const int year, const int m, const int start_wd,
                 const int days, char buf[8][21], const int just_month) {
    /* fill whole buffer with spaces + terminating NUL */
    for (int r=0; r<8; r++) {
        memset(buf[r], ' ', 20);
        buf[r][20] = '\0';
    }

    /* Row 0: month header centered */
    char header[21];
    if (just_month) {
        snprintf(header, sizeof(header), "%s %d", month_names[m], year);
    } else {
        snprintf(header, sizeof(header), "%s", month_names[m]);
    }
    int len = (int)strlen(header);
    int start = (20 - len) / 2;
    memcpy(buf[0] + start, header, len);

    /* Row 1: weekday labels */
    memcpy(buf[1], "Su Mo Tu We Th Fr Sa", 20);

    /* Rows 2–7: days */
    int row = 2;
    int col = start_wd * 3;   /* 3 chars per weekday slot */

    for (int d=1; d<=days; d++) {
        buf[row][col]     = (d < 10) ? ' ' : '0' + d/10;
        buf[row][col + 1] = '0' + (d % 10);
        /* col+2 already a space from memset() */
        col += 3;
        if (col >= 21) { col = 0; row++; }
    }
}

int print_year(const int year) {
    /* Start with the year header */
    printf("                               %i\n", year);
    /* Loop once for each 'season' */
    for (int s = 0; s < 12; s+=3) {
        /* Initialize array for each month in 'season' */
        char month_1_array[8][21];
        char month_2_array[8][21];
        char month_3_array[8][21];
        /* Determine day of week of the first of January */
        int start_day = weekday_sakamoto(year, s+1, 1);
        /* Build first month of season */
        int n_days = month_len[is_leap(year)][s];
        build_month(year, s+0, start_day, n_days, month_1_array, 0);
        /* Build second month of season */
        start_day = (start_day + n_days) % 7;
        n_days = month_len[is_leap(year)][s+1];
        build_month(year, s+1, start_day, n_days, month_2_array, 0);
        /* Build third month of season*/
        start_day = (start_day + n_days) % 7;
        n_days = month_len[is_leap(year)][s+2];
        build_month(year, s+2, start_day, n_days, month_3_array, 0);
        /* Print the season */
        printf("\n");
        for (int r=0; r<8; r++) {
            printf("%s   %s   %s\n", month_1_array[r], month_2_array[r], month_3_array[r]);
        }
    }
    return EXIT_SUCCESS;
}

int print_month(const int year, const int month) {
    /* Initialize empty month array */
    char month_array[8][21];
    /* Determine how many days in the month */
    const int n_days = month_len[is_leap(year)][month];
    /* Determine the first day of the month */
    const int start_day = weekday_sakamoto(year, month+1, 1);
    /* Populate the month array with days */
    build_month(year, month, start_day, n_days, month_array, 1);

    /* Figure out today's date */
    const time_t now = time(NULL);
    const struct tm *t = localtime(&now);
    const int today_y = t->tm_year + 1900;
    const int today_m = t->tm_mon;   /* already zero-indexed */
    const int today_d = t->tm_mday;

    /* Do we highlight? */
    const int highlight = (today_y == year && today_m == month);

    printf("\n");
    for (int r=0; r<8; r++) {
        if (r < 2) {  // header + weekday labels
            printf("%s\n", month_array[r]);
            continue;
        }
        /* special printing loop so we can insert ANSI codes */
        for (int c=0; c<21; c++) {
            /* Detect day numbers: 2 chars wide starting at cols 0,3,6,... */
            if (c % 3 == 0 && isdigit(month_array[r][c])) {
                int day;
                if (isdigit(month_array[r][c+1])) {
                    char buf[3] = {month_array[r][c], month_array[r][c+1], '\0'};
                    day = atoi(buf);
                } else {
                    char buf[2] = {month_array[r][c], '\0'};
                    day = atoi(buf);
                }

                if (highlight && day == today_d) {
                    /* Print highlighted */
                    printf("%s%2d%s", ANSI_BLUE_B, day, ANSI_RESET);
                } else {
                    printf("%2d", day);
                }
                c++; /* we already consumed the next char */
            } else {
                putchar(month_array[r][c]);
            }
        }
        putchar('\n');
    }
    return EXIT_SUCCESS;
}


int main(const int argc, char *argv[]) {
    int year, month;
    int print_mon;

    if (argc == 1) {
        /* no args, use current year and month */
        const time_t now = time(NULL);
        const struct tm *t = localtime(&now);

        year = t->tm_year + 1900;
        month = t->tm_mon + 1;
        print_mon = 1;
    } else if (argc == 2) {
        /* one arg: take a year */
        year = (int)strtol(argv[1], NULL, 10);
        /* month is arbitrary - value not needed */
        month = 1;
        if (year < 1000 || year > 9999) {
            fprintf(stderr, "%s: bad argument: %s%s%s. Year must be 4-digit integer.\n",
                APPNAME, ANSI_RED_B, argv[1], ANSI_RESET);
            return EXIT_FAILURE;
        }
        print_mon = 0;
    } else if (argc == 3) {
        /* two args: take a month and a year */
        month = (int)strtol(argv[1], NULL, 10);
        year = (int)strtol(argv[2], NULL, 10);
        if (year < 1000 || year > 9999) {
            fprintf(stderr, "%s: bad argument: %s%s%s. Year must be 4-digit integer.\n",
                APPNAME, ANSI_RED_B, argv[1], ANSI_RESET);
            return EXIT_FAILURE;
        }
        if (month < 1 || month > 12) {
            fprintf(stderr, "%s: bad argument: %s%s%s. Month must be 1-12 inclusive.\n)",
                APPNAME, ANSI_RED_B, argv[1], ANSI_RESET);
            return EXIT_FAILURE;
        }
        print_mon = 1;
    } else {
        /* three or more args: feel the shame */
        fprintf(stderr, "%s: bad arguments.\n", APPNAME);
        show_help();
        return EXIT_FAILURE;
    }

    /* subtract 1 from month arg for zero-indexing */
    month -= 1;

    if (print_mon) {
        /* We're just printing a single month */
        print_month(year, month);
    } else {
        /* We're going to print a whole year */
        print_year(year);
    }
    return EXIT_SUCCESS;
}
