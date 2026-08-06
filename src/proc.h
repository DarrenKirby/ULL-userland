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

#define USER_W 10
#define PID_W 6
#define CPU_W 5
#define MEM_W 5
#define VSZ_W 10
#define RSS_W 10
#define TTY_W 6
#define STAT_W 4
#define START_W 5
#define TIME_W 5


typedef struct proc_t {
    char     *cmdline;   /* command line arguments */
    char     state;      /* state (R is running, S is sleeping, D is sleeping in an
                            uninterruptible wait, Z is zombie, T is traced or stopped) */
    /*
     * Fields from /proc/<pid>/stat
     *
     */
    int32_t  pid;           /* process id */
    int32_t  ppid;          /* process id of the parent process */
    int32_t  pgrp;          /* pgrp of the process */
    int32_t  sid;           /* session id */
    int32_t  tty_nr;        /* tty the process uses */
    int32_t  tty_pgrp;      /* pgrp of the tty */
    uint64_t flags;         /* task flags */
    uint64_t min_flt;       /* number of minor faults */
    uint64_t cmin_flt;      /* number of minor faults with child’s */
    uint64_t maj_flt;       /* number of major faults */
    uint64_t cmaj_flt;      /* number of major faults with child’s */
    uint64_t utime;         /* user mode jiffies */
    uint64_t stime;         /* kernel mode jiffies */
    uint64_t cutime;        /* user mode jiffies with child’s */
    uint64_t cstime;        /* kernel mode jiffies with child’s */
    int32_t  priority;      /* priority level */
    int32_t  nice;          /* nice level */
    int32_t  num_threads;   /* number of threads */
    uint64_t start_time;    /* time the process started after system boot */
    uint32_t vsize;         /* virtual memory size */
    uint32_t rss;           /* resident set memory size */
    uint32_t rsslim;        /* current limit in bytes on the rss */
    uint32_t start_code;    /* address above which program text can run */
    uint32_t end_code;      /* address below which program text can run */
    uint32_t start_stack;   /* address of the start of the main process stack */
    uint32_t esp;           /* current value of ESP */
    uint32_t eip;           /* current value of EIP */
    int32_t  exit_signal;   /* signal to send to parent thread on exit */
    int32_t  task_cpu;      /* which CPU the task is scheduled on */
    int32_t  rt_priority;   /* realtime priority */
    int32_t  policy;        /* scheduling policy (man sched_setscheduler) */
    uint64_t blkio_ticks;   /* time spent waiting for block IO */
    uint64_t gtime;         /* guest time of the task in jiffies */
    uint64_t cgtime;        /* guest time of the task children in jiffies */
    /*
     * Fields from /proc/<pid>/status
     *
     */
    char     *name;       /* filename of the executable */
    uint32_t tgid;        /* thread group ID */
    uint32_t ngid;        /* NUMA group ID (0 if none) */
    uid_t    ruid;        /* real uid */
    uid_t    euid;        /* effective uid */
    uid_t    suid;        /* saved set uid */
    uid_t    fuid;        /* file system uid */
    gid_t    rgid;        /* real gid */
    gid_t    egid;        /* effective gid */
    gid_t    sgid;        /* saved set gid */
    gid_t    fgid;        /* flie system gid */
    uint32_t fd_size;     /* number of file descriptor slots currently allocated */
    bool     k_thread;    /* kernel thread flag, true is yes, false is no */
    uint32_t vm_peak;     /* peak virtual memory size */
    uint32_t vm_size;     /* total program size */
    uint32_t vm_lck;      /* locked memory size */
    uint32_t vm_pin;      /* pinned memory size */
    uint32_t vm_hwm;      /* peak resident set size (“high water mark”) */
    uint32_t vm_rss;      /* size of memory portions. VmRSS = RssAnon + RssFile + RssShmem. */
    uint32_t rss_anon;    /* size of resident anonymous memory */
    uint32_t rss_file;    /* size of resident file mappings */
    uint32_t rss_shmem;   /* size of resident shmem memory */
    uint32_t vm_data;     /* size of private data segments */
    uint32_t vm_stk;      /* size of stack segments */
    uint32_t vm_exe;      /* size of text segment */
    uint32_t vm_lib;      /* size of shared library code */
    uint32_t vm_pte;      /* size of page table entries */
    uint32_t vm_swap;     /* amount of swap used by anonymous private data */
} proc_t;


/* Declare a global array of proc_t structs. */
proc_t **proc_array;

int is_pid_dir(const char *name);


#endif