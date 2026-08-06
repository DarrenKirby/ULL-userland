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
#include <getopt.h>
#include <sys/sysmacros.h>

#include "proc.h"
#include "common.h"


static const char *APP_NAME = "ps";

static struct {
    bool cur_tty;
    bool cur_uid;
    bool con_term;
} opts = { .cur_tty = true, .cur_uid = true, .con_term = false };

static void show_help()
{
    printf("Usage: %s [OPTION]...\n\n\
    Report a snapshot of the current processes\n\n\
Options:\n\
    -a, --all\t print processes from all TTYs\n\
    -e, --everyone\t print processes owned by all UID's\n\
Report bugs to <darren@dragonbyte.ca>\n", APP_NAME);
}

size_t get_n_processes()
{
    size_t n_proc = 0;
    /* Open the /proc directory to iterate over all active processes. */
    DIR *proc_dir = opendir("/proc");
    if (!proc_dir) {
        perror("Failed to open /proc");
        return EXIT_FAILURE;
    }

    struct dirent *entry;
    while ((entry = readdir(proc_dir)) != NULL) {
        /* Skip non-process directories. */
        if (!is_pid_dir(entry->d_name)) continue;
        n_proc++;
    }

    closedir(proc_dir);
    return n_proc;
}

/* Helper function to check if a directory name is purely numeric (a PID). */
int is_pid_dir(const char *name)
{
    for (int i = 0; name[i] != '\0'; i++) {
        if (!isdigit((unsigned char)name[i])) return 0;
    }
    return 1;
}

/* Returns the device number ssociated with the calling
 * process' tty. */
int get_current_tty_nr()
{
    struct stat tty_stat;

    /* Verify if standard input is attached to a terminal. */
    if (!isatty(STDIN_FILENO)) {
        perror("Standard input is not a TTY");
        return -1;
    }

    /* Retrieve file status metadata for the TTY. */
    if (fstat(STDIN_FILENO, &tty_stat) == -1) {
        perror("Failed to stat TTY");
        return -1;
    }

    return (int)tty_stat.st_rdev;
}

/* Constructs a 32-bit integer from 4 characters (Little-Endian). */
#define U32_KEY(a, b, c, d) \
((uint32_t)(a) | ((uint32_t)(b) << 8) | ((uint32_t)(c) << 16) | ((uint32_t)(d) << 24))

void read_proc_status(char *s, proc_t *p)
{
    char *line = s;

    while (line && *line) {
        /* Ensure we have at least 4 characters before the end of the string/line. */
        if (!line[0] || !line[1] || !line[2] || !line[3]) {
            /* Out of data. */
            return;
        }

        uint32_t prefix;
        memcpy(&prefix, line, 4);
        char *tmp;
        char buf[64];
        int i;

        switch (prefix) {
            case U32_KEY('N', 'a', 'm', 'e'):
                line += 6; /* Advance to the \t */
                i = 0;
                tmp = line;
                while (*tmp != '\n') {
                    buf[i++] = *tmp++;
                }
                buf[i] = '\0';
                p->name = strdup(buf);
                break;
            case U32_KEY('U', 'i', 'd', ':'):
                line +=5; /* advance to first uid */
                p->euid = strtoul(line, &line, 10);
                p->ruid = strtoul(line, &line, 10);
                p->suid = strtoul(line, &line, 10);
                p->fuid = strtoul(line, &line, 10);
                break;
            case U32_KEY('G', 'i', 'd', ':'):
                line +=5; /* advance to first gid */
                p->egid = strtoul(line, &line, 10);
                p->rgid = strtoul(line, &line, 10);
                p->sgid = strtoul(line, &line, 10);
                p->fgid = strtoul(line, &line, 10);
                break;
            case U32_KEY('K', 't', 'h', 'r'):
                line += 9; /* position at arg */
                p->k_thread = (*line == '0') ? false : true;
                break;
            case U32_KEY('V', 'm', 'S', 'i'):
                line = strchr(line, '\t');
                p->vm_size = strtoul(line, nullptr, 10);
                break;
            case U32_KEY('V', 'm', 'P', 'e'):
                line = strchr(line, '\t');
                p->vm_peak = strtoul(line, nullptr, 10);
                break;
            case U32_KEY('V', 'm', 'L', 'c'):
                line = strchr(line, '\t');
                p->vm_lck = strtoul(line, nullptr, 10);
                break;
            case U32_KEY('V', 'm', 'P', 'i'):
                line = strchr(line, '\t');
                p->vm_pin = strtoul(line, nullptr, 10);
                break;
            case U32_KEY('V', 'm', 'H', 'W'):
                line = strchr(line, '\t');
                p->vm_hwm = strtoul(line, nullptr, 10);
                break;
            case U32_KEY('V', 'm', 'R', 'S'):
                line = strchr(line, '\t');
                p->vm_rss = strtoul(line, nullptr, 10);
                break;
            case U32_KEY('R', 's', 's', 'A'):
                line = strchr(line, '\t');
                p->rss_anon = strtoul(line, nullptr, 10);
                break;
            case U32_KEY('R', 's', 's', 'F'):
                line = strchr(line, '\t');
                p->rss_file = strtoul(line, nullptr, 10);
                break;
            case U32_KEY('R', 's', 's', 'S'):
                line = strchr(line, '\t');
                p->rss_shmem = strtoul(line, nullptr, 10);
                break;
            case U32_KEY('V', 'm', 'D', 'a'):
                line = strchr(line, '\t');
                p->vm_data = strtoul(line, nullptr, 10);
                break;
            case U32_KEY('V', 'm', 'S', 't'):
                line = strchr(line, '\t');
                p->vm_stk = strtoul(line, nullptr, 10);
                break;
            case U32_KEY('V', 'm', 'E', 'x'):
                line = strchr(line, '\t');
                p->vm_exe = strtoul(line, nullptr, 10);
                break;
            case U32_KEY('V', 'm', 'L', 'i'):
                line = strchr(line, '\t');
                p->vm_lib = strtoul(line, nullptr, 10);
                break;
            case U32_KEY('V', 'm', 'P', 'T'):
                line = strchr(line, '\t');
                p->vm_pte = strtoul(line, nullptr, 10);
                break;
            case U32_KEY('V', 'm', 'S', 'w'):
                line = strchr(line, '\t');
                p->vm_swap = strtoul(line, nullptr, 10);
                break;
            default:
                /* Uninteresting field, do nothing. */
                break;
        }

        /* Advance to the next line. */
        line = strchr(line, '\n');
        if (line) line++;
    }
}

void read_proc_stat(char *s, proc_t *p) {
    char *tmp = strchr(s, ')');
    s = tmp + 2; /* Skip straight to state. */
    p->state = *s++; /* Grab the single char */
    p->ppid = (int)strtol(s, &s, 10);
    p->pgrp = (int)strtol(s, &s, 10);
    p->sid = (int)strtol(s, &s, 10);
    p->tty_nr = (int)strtol(s, &s, 10);
    p->tty_pgrp = (int)strtol(s, &s, 10);
    p->flags = strtoull(s, &s, 10);
    p->min_flt = strtoull(s, &s, 10);
    p->cmin_flt = strtoull(s, &s, 10);
    p->maj_flt = strtoull(s, &s, 10);
    p->cmaj_flt = strtoull(s, &s, 10);
    p->utime = strtoull(s, &s, 10);
    p->stime = strtoull(s, &s, 10);
    p->cutime = strtoull(s, &s, 10);
    p->cstime = strtoull(s, &s, 10);
    p->priority = (int)strtol(s, &s, 10);
    p->nice = (int)strtol(s, &s, 10);
    p->num_threads = (int)strtol(s, &s, 10);
    /* it_real_value - obsolete - always 0 */
    strtol(s, &s, 10); /* discard */
    p->start_time = strtoull(s, &s, 10);
    p->vsize = strtoul(s, &s, 10);
    p->rss = strtoul(s, &s, 10);
    p->rsslim = strtoul(s, &s, 10);
    p->start_code = strtoul(s, &s, 10);
    p->end_code = strtoul(s, &s, 10);
    p->start_stack = strtoul(s, &s, 10);
    p->esp = strtoul(s, &s, 10);
    p->eip = strtoul(s, &s, 10);
    /* pending, blocked, sigign, sigcatch */
    strtol(s, &s, 10); /* discard */
    strtol(s, &s, 10); /* discard */
    strtol(s, &s, 10); /* discard */
    strtol(s, &s, 10); /* discard */
    /* Three discarded placeholders */
    strtol(s, &s, 10); /* discard */
    strtol(s, &s, 10); /* discard */
    strtol(s, &s, 10); /* discard */
    p->exit_signal = (int)strtol(s, &s, 10);
    p->task_cpu = (int)strtol(s, &s, 10);
    p->rt_priority = (int)strtol(s, &s, 10);
    p->policy = (int)strtol(s, &s, 10);
    p->blkio_ticks = strtoull(s, &s, 10);
    p->gtime = strtoull(s, &s, 10);
    p->cgtime = strtoull(s, &s, 10);
}

/* This function iterates over the process directories in /proc.
 * The arguments are flags which indicate which processes we are
 * interested in:
 *  cur_tty: only print proc's attatched to the current tty
 *  cur_uid: only print proc's owned by the urrent euid
 *  con_term: only print proc's with a controlling term
 */
int parse_processes(const bool cur_tty, const bool curr_uid)
{
    int target_tty = 0;
    uid_t target_uid = UINT32_MAX; /* Not likely to be valid euid. */

    if (cur_tty) {
        if ((target_tty = get_current_tty_nr()) == -1) {
            fprintf(stderr, "Failed to get current tty");
        }
    }

    if (curr_uid) {
        target_uid = geteuid();
    }

    /* Open the /proc directory to iterate over all active processes. */
    DIR *proc_dir = opendir("/proc");
    if (!proc_dir) {
        perror("Failed to open /proc");
        return EXIT_FAILURE;
    }

    char stat_path[1024];
    int a_idx = 0;

    char *read_buf = malloc(READ_BUF_SIZE);
    if (!read_buf) {
        fprintf(stderr, "ENOMEM: Failed to allocate read buffer\n");
        exit(EXIT_FAILURE);
    }

    struct dirent *entry;
    while ((entry = readdir(proc_dir)) != NULL) {
        /* Skip non-process directories. */
        if (!is_pid_dir(entry->d_name)) continue;

        /* Allocate the proc_t struct for THIS process. */
        proc_t *proc_stat = calloc(1, sizeof(proc_t));
        if (!proc_stat) {
            fprintf(stderr, "ENOMEM: Failed to allocate proc stat\n");
            exit(EXIT_FAILURE);
        }

        /* First read is /proc/<pid>/stat. */
        snprintf(stat_path, sizeof(stat_path), "/proc/%s/stat", entry->d_name);
        FILE *f = fopen(stat_path, "r");
        if (!f) {
            /* Not an error: process might have terminated. */
            free(proc_stat);
            continue;
        }

        size_t n_read = fread(read_buf, 1, READ_BUF_SIZE - 1, f);
        if (n_read == 0 && ferror(f)) {
            fprintf(stderr, "Failed to read from /proc/%s/stat", entry->d_name);
            exit(EXIT_FAILURE);
        }
        read_buf[n_read] = '\0'; /* Null-terminate the string */
        fclose(f);

        /* Copy the pid. */
        proc_stat->pid = (int32_t)strtol(entry->d_name, nullptr, 10);
        /* Parse the file. */
        read_proc_stat(read_buf, proc_stat);

        /* Ensure we even want this one. */
        if (cur_tty && target_tty != proc_stat->tty_nr) {
            free(proc_stat);
            continue;
        }

        /* Next read is /proc/<pid>/status. */
        snprintf(stat_path, sizeof(stat_path), "/proc/%s/status", entry->d_name);
        f = fopen(stat_path, "r");
        if (!f) {
            free(proc_stat);
            continue;
        }

        n_read = fread(read_buf, 1, READ_BUF_SIZE - 1, f);
        if (n_read == 0 && ferror(f)) {
            fprintf(stderr, "Failed to read from /proc/%s/status", entry->d_name);
            exit(EXIT_FAILURE);
        }
        read_buf[n_read] = '\0'; /* Null-terminate the string */
        fclose(f);

        /* Parse the file. */
        read_proc_status(read_buf, proc_stat);

        /* Ensure we even want this one. */
        if (curr_uid && target_uid != proc_stat->euid) {
            free(proc_stat);
            continue;
        }

        /* Next read is /proc/<pid>/cmdline. */
        snprintf(stat_path, sizeof(stat_path), "/proc/%s/cmdline", entry->d_name);
        f = fopen(stat_path, "r");
        if (!f) {
            free(proc_stat);
            continue;
        }

        n_read = fread(read_buf, 1, READ_BUF_SIZE - 1, f);
        if (n_read == 0 && ferror(f)) {
            fprintf(stderr, "Failed to read from /proc/%s/cmdline", entry->d_name);
            exit(EXIT_FAILURE);
        }
        read_buf[n_read] = '\0'; /* Null-terminate the string */
        fclose(f);

        /* Parse the file. */
        proc_stat->cmdline = strdup(read_buf);

        /* Done parsing. Push the proc_t into the array. */
        proc_array[a_idx++] = proc_stat;
    }

    proc_array[a_idx] = nullptr;
    closedir(proc_dir);
    free(read_buf);
    return 0;
}

long double get_uptime_s()
{
    FILE* f = fopen("/proc/uptime", "r");
    if (!f) {
        fprintf(stderr, "Failed to open /proc/uptime");
        exit(EXIT_FAILURE);
    }

    char buf[16];
    const size_t n_read = fread(buf, 1, sizeof(buf), f);
    if (n_read == 0) {
        fprintf(stderr, "Failed to read /proc/uptime");
        exit(EXIT_FAILURE);
    }
    buf[n_read] = '\0';

    return strtold(buf, nullptr);
}

uint64_t get_mem_total()
{
    FILE* f = fopen("/proc/meminfo", "r");
    if (!f) {
        fprintf(stderr, "Failed to open /proc/meminfo");
        exit(EXIT_FAILURE);
    }

    char *line = nullptr;
    size_t size = 0;
    const ssize_t n_read = getline(&line, &size, f);
    if (n_read == -1) {
        fprintf(stderr, "Failed to read /proc/meminfo");
        free(line);
        fclose(f);
        exit(EXIT_FAILURE);
    }

    char *tmp = strchr(line, ':');
    if (tmp) {
        tmp++;
    } else {
        tmp = line;
    }

    const uint64_t r = strtoull(tmp, nullptr, 10);

    free(line);
    fclose(f);

    return r;
}

uint32_t get_hertz()
{
    return sysconf(_SC_CLK_TCK);
}

long double get_cpu_percent(const uint32_t h, const long double su, const uint64_t u,
                            const uint64_t s, const uint64_t sta)
{
    const long double exec_t = (u + s) / (long double)h;
    const long double proc_t = su - (sta / (long double)h);
    return (exec_t / proc_t) * 100;
}

long double get_mem_percent(const uint64_t mt, const uint64_t pt) {
    return (pt / (long double)mt) * 100;
}

char* get_tty(uint32_t device, char buf[]) {
    unsigned int maj = major(device);
    unsigned int min = minor(device);

    if (maj == 4 && min > 0 && min < 64) {
        snprintf(buf, 16, "tty%d", maj);
        return buf;
    }

    if (maj > 135 && maj < 144) {
        snprintf(buf, 16, "pts/%d", min);
        return buf;
    }

    snprintf(buf, 16, "?");
    return buf;
}

void print_processes() {

    /* Print the header. */
    printf("%-*s ", USER_W, "USER");
    printf("%*s ",  PID_W,  "PID");
    printf("%*s ",  CPU_W,  "%CPU");
    printf("%*s ",  MEM_W,  "%MEM");
    printf("%*s ",  VSZ_W,  "VSZ");
    printf("%*s ",  RSS_W,  "RSS");
    printf("%*s ",  TTY_W,  "TTY");
    printf("%*s ",  STAT_W, "STAT");
    printf("%s\n", "COMMAND");

    const uint32_t hertz = get_hertz();
    const long double sys_uptime = get_uptime_s();
    const uint64_t mem_t = get_mem_total();
    char tty_buf[16];

    while (*proc_array != nullptr) {
        const proc_t *proc_stat = *proc_array;

        printf("%-*s ", USER_W, get_username(proc_stat->euid));
        printf("%*u ",  PID_W,  proc_stat->pid);
        printf("%5.1Lf ", get_cpu_percent(hertz, sys_uptime,
            proc_stat->utime, proc_stat->stime, proc_stat->start_time));
        printf("%5.1Lf ", get_mem_percent(mem_t, proc_stat->vm_rss));
        printf("%*u ",  VSZ_W,  proc_stat->vm_size);
        printf("%*u ",  RSS_W,  proc_stat->vm_rss);
        get_tty(proc_stat->tty_nr, tty_buf);
        printf("%*s ",  TTY_W,  tty_buf);
        printf("%*c ",  STAT_W, proc_stat->state);
        if (proc_stat->k_thread) {
            printf("[%s]\n", proc_stat->name);
        } else {
            printf("%s\n", proc_stat->cmdline);
        }


        //
        //
        // printf("name: '%s' ", proc_stat->name);
        // printf("cmdline: '%s' ", proc_stat->cmdline);
        // printf("kthread: %d ", proc_stat->k_thread);
        // printf("vm exe: %u ", proc_stat->vm_exe);
        // printf("vm hwm: %u ", proc_stat->vm_hwm);
        // printf("vm lck: %u ", proc_stat->vm_lck);
        // printf("vm peak: %u ", proc_stat->vm_peak);
        // printf("vm lib: %u ", proc_stat->vm_lib);
        // printf("vm data: %u ", proc_stat->vm_data);
        // printf("vm rss: %u ", proc_stat->vm_rss);
        // printf("vm size: %u ", proc_stat->vm_size);
        //
        // printf("euid: %u ", proc_stat->euid);
        // printf("ruid: %u ", proc_stat->ruid);
        // printf("suid: %u ", proc_stat->suid);
        // printf("fuid: %u ", proc_stat->fuid);
        // printf("egid: %u ", proc_stat->egid);
        // printf("rgid: %u ", proc_stat->rgid);
        // printf("sgid: %u ", proc_stat->sgid);
        // printf("fgid: %u ", proc_stat->fgid);
        //
        // printf("pid: %d ", proc_stat->pid);
        // printf("state: %c ", proc_stat->state);
        // printf("ppid: %d ", proc_stat->ppid);
        // printf("pgrp: %d ", proc_stat->pgrp);
        // printf("sid: %d ", proc_stat->sid);
        // printf("tty_nr: %d ", proc_stat->tty_nr);
        // printf("tty_pgrp: %d ", proc_stat->tty_pgrp);
        // printf("flags: %lu ", proc_stat->flags);
        // printf("min_flt: %lu ", proc_stat->min_flt);
        // printf("cmin_flt: %lu ", proc_stat->cmin_flt);
        // printf("maj_flt: %lu ", proc_stat->maj_flt);
        // printf("cmaj_flt: %lu ", proc_stat->cmaj_flt);
        // printf("utime: %lu ", proc_stat->utime);
        // printf("stime: %lu ", proc_stat->stime);
        // printf("cstime: %lu ", proc_stat->cstime);
        // printf("priority: %d ", proc_stat->priority);
        // printf("nice: %d ", proc_stat->nice);
        // printf("num_threads: %d ", proc_stat->num_threads);
        // printf("start_time: %lu ", proc_stat->start_time);
        // printf("vsize: %u ", proc_stat->vsize);
        // printf("rss: %u ", proc_stat->rss);
        // printf("rsslim: %u ", proc_stat->rsslim);
        // printf("start_code: %u ", proc_stat->start_code);
        // printf("end_code: %u ", proc_stat->end_code);
        // printf("start_stack: %u ", proc_stat->start_stack);
        // printf("esp: %u ", proc_stat->esp);
        // printf("eip: %u ", proc_stat->eip);
        // printf("exit_signal: %u ", proc_stat->exit_signal);
        // printf("task_cpu: %u ", proc_stat->task_cpu);
        // printf("rt_priority: %u ", proc_stat->rt_priority);
        // printf("policy: %u ", proc_stat->policy);
        // printf("blkio_ticks: %lu ", proc_stat->blkio_ticks);
        // printf("cgtime: %lu ", proc_stat->cgtime);
        // printf("\n\n");

        proc_array++;
    }
}


int main(const int argc, char** argv ) {

    const struct option longopts[] = {
        { .name = "help",       .has_arg = no_argument,       .flag = nullptr, .val = 'h' },
        { .name = "version",    .has_arg = no_argument,       .flag = nullptr, .val = 'V' },
        { .name = "all",     .has_arg = no_argument,       .flag = nullptr, .val = 'a' },
        { .name = "everyone",  .has_arg = no_argument,       .flag = nullptr, .val = 'e' },
        { .name = nullptr,      .has_arg = no_argument,       .flag = nullptr, .val = 0 }
    };

    int opt;
    while ((opt = getopt_long(argc, argv, "Vhae", longopts, nullptr)) != -1) {
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
                opts.cur_tty = false;
                break;
            case 'e':
                opts.cur_uid = false;
                break;
            default:
                show_help();
                EXIT_FAILURE;
                break;
        }
    }

    const size_t arr_size = get_n_processes();
    proc_array = (proc_t**)malloc((arr_size + 1) * sizeof(proc_t*));
    if (!proc_array) {
        fprintf(stderr, "ENOMEM: Failed to allocate proc array\n");
        exit(EXIT_FAILURE);
    }

    parse_processes(opts.cur_tty, opts.cur_uid);
    print_processes();

    return EXIT_SUCCESS;
}