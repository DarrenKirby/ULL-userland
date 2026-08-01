/***************************************************************************
 *   groups - print the groups a user is in                                *
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
#include <grp.h>

#include "common.h"

/* Linux getgrouplist wants a gid_t,
 * stupid macOS wants an int. */
#ifdef __linux__
typedef gid_t gid;
#else
typedef int gid;
#endif

#ifndef NGROUPS_MAX
#define NGROUPS_MAX 32
#endif


static const char *APP_NAME = "groups";

static void show_help()
{
    printf("Usage: %s [OPTION]...\n\n\
Options:\n\
    -h, --help\t\t display this help\n\
    -V, --version\t display version information\n\n\
Report bugs to <darren@dragonbyte.ca>\n", APP_NAME);
}

int main(const int argc, char *argv[])
{
    const struct option long_opts[] = {
        {.name = "help",    .has_arg = no_argument, .flag = nullptr, .val = 'h'},
        {.name = "version", .has_arg = no_argument, .flag = nullptr, .val = 'V'},
        {.name = nullptr,   .has_arg = no_argument, .flag = nullptr, .val = 0}
    };

    int opt;
    while ((opt = getopt_long(argc, argv, "Vh", long_opts, nullptr)) != -1) {
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
            default:
                show_help();
                return EXIT_FAILURE;
        }
    }

    /* No arguments. */
    if (argc == optind) {
        gid_t *groups = malloc(sizeof(gid_t) * NGROUPS_MAX);
        const int n_groups = getgroups(NGROUPS_MAX, groups);

        for (int i = 0; i < n_groups; i++) {
            printf("%s ", get_groupname(groups[i]));
        }
        printf("\n");
        free(groups);
    } else {
        /* Iterate over arguments. */
        gid *groups = malloc(sizeof(int) * NGROUPS_MAX);
        while (optind < argc) {
            const struct passwd *pwd = getpwnam(argv[optind]);

            /* Check if the user exists. */
            if (!pwd) {
                fprintf(stderr, "%s: No such user: '%s'\n", APP_NAME, argv[optind++]);
                continue;
            }

            int n_groups = NGROUPS_MAX;
            if (getgrouplist(argv[optind], pwd->pw_gid, groups, &n_groups) == -1) {
                fprintf(stderr, "%s: Too many groups to fit in array! Groups list truncated!\n", APP_NAME);
            }

            printf("%s: ", argv[optind++]);

            for (int i = 0; i < n_groups; i++) {
                printf("%s ", get_groupname(groups[i]));
            }
            printf("\n");

            /* Zero out the buffer. */
            memset(groups, 0, sizeof(int) * NGROUPS_MAX);
        }
        free(groups);
    }

    return EXIT_SUCCESS;
}
