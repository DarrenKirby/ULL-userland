/***************************************************************************
 *   proc.h - datatypes and routines common to process information         *
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

#ifndef PROC_H
#define PROC_H

#include <stdint.h>

#define READ_BUF_SIZE 4096

typedef struct proc_t {
    char *cmdline;
    char state;
    /* Fields from /proc/<pid>/stat */
    int32_t pid;
    int32_t ppid;
    int32_t pgrp;
    int32_t sid;
    int32_t tty_nr;
    int32_t tty_pgrp;
    uint64_t flags;
    uint64_t min_flt;
    uint64_t cmin_flt;
    uint64_t maj_flt;
    uint64_t cmaj_flt;
    uint64_t utime;
    uint64_t stime;
    uint64_t cutime;
    uint64_t cstime;
    int32_t priority;
    int32_t nice;
    int32_t num_threads;
    uint64_t start_time;
    uint32_t vsize;
    uint32_t rss;
    uint32_t rsslim;
    uint32_t start_code;
    uint32_t end_code;
    uint32_t start_stack;
    uint32_t esp;
    uint32_t eip;
    int32_t exit_signal;
    int32_t task_cpu;
    int32_t rt_priority;
    int32_t policy;
    uint64_t blkio_ticks;
    uint64_t gtime;
    uint64_t cgtime;
    /* Fields from /proc/<pid>/status */
    char *name;
    uint32_t tgid;
    uint32_t ngid;
    uid_t euid;
    uid_t ruid;
    uid_t suid;
    uid_t fuid;
    gid_t egid;
    gid_t rgid;
    gid_t sgid;
    gid_t fgid;
    uint32_t fd_size;
    bool k_thread;
    uint32_t vm_peak;
    uint32_t vm_size;
    uint32_t vm_lck;
    uint32_t vm_pin;
    uint32_t vm_hwm;
    uint32_t vm_rss;
    uint32_t rss_anon;
    uint32_t rss_file;
    uint32_t rss_shmem;
    uint32_t vm_data;
    uint32_t vm_stk;
    uint32_t vm_exe;
    uint32_t vm_lib;
    uint32_t vm_pte;
    uint32_t vm_swap;



} proc_t;


/* Declare a global array of proc_t structs. */
proc_t **proc_array;

int is_pid_dir(const char *name);


#endif