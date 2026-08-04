/***************************************************************************
 *   proc.c - function definitions common to process information           *
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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>
#include <ctype.h>
#include <unistd.h>

// Helper function to check if a directory name is purely numeric (a PID)
int is_pid_dir(const char *name) {
    for (int i = 0; name[i] != '\0'; i++) {
        if (!isdigit((unsigned char)name[i])) return 0;
    }
    return 1;
}


int main() {
    struct stat tty_stat;

    // 1. Get the device ID (st_rdev) of the current controlling terminal
    // Replace "/dev/tty" with a specific path (e.g., "/dev/pts/1") if targeting another terminal
    if (stat("/dev/tty", &tty_stat) == -1) {
        perror("Failed to stat terminal");
        return EXIT_FAILURE;
    }
    int target_tty = (int)tty_stat.st_rdev;
    printf("Searching for processes on terminal device ID: %d\n", target_tty);
    printf("%-8s %s\n", "PID", "COMMAND");
    printf("---------------------\n");

    // 2. Open the /proc directory to iterate over all active processes
    DIR *proc_dir = opendir("/proc");
    if (!proc_dir) {
        perror("Failed to open /proc");
        return EXIT_FAILURE;
    }

    struct dirent *entry;
    while ((entry = readdir(proc_dir)) != NULL) {
        // Skip non-process directories
        if (!is_pid_dir(entry->d_name)) continue;

        // Build path to /proc/[PID]/stat
        char stat_path[256];
        snprintf(stat_path, sizeof(stat_path), "/proc/%s/stat", entry->d_name);

        FILE *f = fopen(stat_path, "r");
        if (!f) continue; // Process might have terminated

        int pid;
        char comm[256];
        char state;
        int ppid, pgrp, session, tty_nr;

        // 3. Parse the /proc/[PID]/stat file
        // Format: PID (filename) State PPID PGRP SESSION TTY_NR ...
        // We use " %[^)]s" to securely grab everything inside the parentheses for the command name
        if (fscanf(f, "%d (%[^)]) %c %d %d %d %d", &pid, comm, &state, &ppid, &pgrp, &session, &tty_nr) == 7) {
            // 4. Check if the process's tty matches our target terminal's device number
            if (tty_nr == target_tty) {
                printf("%-8d %s\n", pid, comm);
            }
        }
        fclose(f);
    }

    closedir(proc_dir);
    return EXIT_SUCCESS;
}