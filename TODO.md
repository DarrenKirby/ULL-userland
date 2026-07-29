### Current status of programs

It looks like I'm picking this up again after nine years.
Now as then, this is just an exercise to teach myself C.

These programs are tested on three systems:
Gentoo Linux (GCC 14.3.0)
OS X Sonoma 14.7.1 (CLANG 16.0.0, GCC 15.1.0)
FreeBSD 14.3-RELEASE (CLANG 14.1.7)

I'll putter around as I find time to do so. In the meantime,
I've decided to keep my original note from 2014 for historic
and sentimental reasons:

=======================================================================

Note: Just because the status is 'completed' doesn't mean I won't
still hack on it. Just means the core functionality that I intend
to implement is there. Over time, I will add more error-checking or
refactoring, as well as adding portability fixes.

On portability: these programs are tested on three systems:
Gentoo Linux       (GCC 4.7.3, GLIBC 2.19)
OS X Mavericks     (LLVM 5.1, CLANG 503.0.40)
FreeBSD 10-Release (CLANGBSD 3.3)

...but developed on the Linux machine for the most part. I am working
on portability but a lot of the apps are flaky or don't even compile
on macOS and FreeBSD. See notes below...

========================================================================

| Name       | Status      | Linux | macOS | FreeBSD | Notes                                      |
|------------|-------------|:-----:|:-----:|:-------:|--------------------------------------------|
| arch       | completed   |  ✅   |  ✅   |   ❌    |
| b2sum      | not started |  ❌   |  ❌   |   ❌    |
| base32     | completed   |  ✅   |  ✅   |   ✅    |
| base64     | not started |  ✅   |  ✅   |   ✅    |
| basename   | completed   |  ✅   |  ✅   |   ✅    |
| basenc     | not started |  ❌   |  ❌   |   ❌    |
| cal        | in progress |  ✅   |  ✅   |   ✅    | Not all options complete                   |
| cat        | completed   |  ✅   |  ✅   |   ✅    |
| chcon      | not started |  ❌   |  ❌   |   ❌    |
| chgrp      | completed   |  ✅   |  ✅   |   ✅    |
| chmod      | not started |  ❌   |  ❌   |   ❌    |
| chown      | completed   |  ✅   |  ✅   |   ✅    |
| chroot     | in progress |  ❌   |  ❌   |   ❌    |
| cksum      | not started |  ❌   |  ❌   |   ❌    |
| comm       | not started |  ❌   |  ❌   |   ❌    |
| cp         | in progress |  ✅   |  ✅   |   ❌    |
| csplit     | not started |  ❌   |  ❌   |   ❌    |
| cut        | not started |  ❌   |  ❌   |   ❌    |
| date       | not started |  ❌   |  ❌   |   ❌    |
| dd         | not started |  ❌   |  ❌   |   ❌    |
| df         | in progress |  ❌   |  ✅   |   ✅    | 
| dir        | not started |  ❌   |  ❌   |   ❌    |
| dircolors  | not started |  ❌   |  ❌   |   ❌    |
| dirname    | in progress |  ✅   |  ✅   |   ✅    |
| dmesg      | not started |  ❌   |  ❌   |   ❌    |
| domainname | not started |  ❌   |  ❌   |   ❌    |
| du         | not started |  ❌   |  ❌   |   ❌    |
| echo       | not started |  ❌   |  ❌   |   ❌    |
| env        | in progress |  ❌   |  ❌   |   ✅    | Printing complete; setting, not so much    |
| expand     | not started |  ❌   |  ❌   |   ❌    |
| expr       | not started |  ❌   |  ❌   |   ❌    |
| factor     | not started |  ❌   |  ❌   |   ❌    |
| false      | completed   |  ✅   |  ✅   |   ✅    |
| fmt        | not started |  ❌   |  ❌   |   ❌    |
| fold       | in progress |  ✅   |  ✅   |   ❌    | Working, but will break on multi-col chars |
| free       | in progress |  ✅   |  ✅   |   ❌    |
| groups     | not started |  ❌   |  ❌   |   ❌    |
| head       | completed   |  ✅   |  ✅   |   ✅    |
| hostid     | not started |  ❌   |  ❌   |   ❌    |
| hostname   | not started |  ❌   |  ❌   |   ❌    |
| id         | not started |  ❌   |  ❌   |   ❌    |
| install    | not started |  ❌   |  ❌   |   ❌    |
| join       | not started |  ❌   |  ❌   |   ❌    |
| kill       | not started |  ❌   |  ❌   |   ❌    |
| link       | in progress |  ✅   |  ✅   |   ✅    |
| ln         | in progress |  ✅   |  ✅   |   ✅    |
| logname    | completed   |  ✅   |  ✅   |   ❌    |
| ls         | in progress |  ✅   |  ✅   |   ✅    |
| md5sum     | in progress |  ✅   |  ✅   |   ✅    |
| mkdir      | completed   |  ✅   |  ✅   |   ✅    |
| mkfifo     | not started |  ❌   |  ❌   |   ❌    |
| mknod      | not started |  ❌   |  ❌   |   ❌    |
| mktemp     | not started |  ❌   |  ❌   |   ❌    |
| mount      | in progress |  ❌   |  ✅   |   ✅    |
| mv         | completed   |  ✅   |  ✅   |   ✅    |
| nice       | not started |  ❌   |  ❌   |   ❌    |
| nl         | in progress |  ✅   |  ✅   |   ✅    |
| nohup      | not started |  ❌   |  ❌   |   ❌    |
| nproc      | completed   |  ✅   |  ✅   |   ❌    |
| numfmt     | not started |  ❌   |  ❌   |   ❌    |
| od         | in progress |  ❌   |  ❌   |   ❌    |
| paste      | not started |  ❌   |  ❌   |   ❌    |
| pathchk    | not started |  ❌   |  ❌   |   ❌    |
| pinky      | not started |  ❌   |  ❌   |   ❌    |
| pr         | not started |  ❌   |  ❌   |   ❌    |
| ps         | not started |  ❌   |  ❌   |   ❌    |
| printenv   | completed   |  ✅   |  ✅   |   ✅    |
| printf     | not started |  ❌   |  ❌   |   ❌    |
| ptx        | not started |  ❌   |  ❌   |   ❌    |
| pwd        | completed   |  ✅   |  ✅   |   ✅    |
| readlink   | not started |  ❌   |  ❌   |   ❌    |
| realpath   | not started |  ❌   |  ❌   |   ❌    |
| renice     | not started |  ❌   |  ❌   |   ❌    |
| rm         | completed   |  ✅   |  ✅   |   ✅    |
| rmdir      | completed   |  ✅   |  ✅   |   ✅    |
| route      | not started |  ❌   |  ❌   |   ❌    |
| seq        | not started |  ❌   |  ❌   |   ❌    |
| sha1sum    | not started |  ❌   |  ❌   |   ❌    |
| sha224sum  | in progress |  ✅   |  ✅   |   ✅    |
| sha256sum  | in progress |  ✅   |  ✅   |   ✅    |
| sha384sum  | in progress |  ✅   |  ✅   |   ✅    |
| sha512sum  | in progress |  ✅   |  ✅   |   ✅    |
| shred      | not started |  ❌   |  ❌   |   ❌    |
| shuf       | not started |  ❌   |  ❌   |   ❌    |
| sleep      | completed   |  ✅   |  ✅   |   ✅    |
| sort       | not started |  ❌   |  ❌   |   ❌    |
| split      | not started |  ❌   |  ❌   |   ❌    |
| stat       | completed   |  ✅   |  ✅   |   ✅    |
| stty       | not started |  ❌   |  ❌   |   ❌    |
| sum        | not started |  ❌   |  ❌   |   ❌    |
| sync       | completed   |  ✅   |  ✅   |   ✅    |
| tac        | not started |  ❌   |  ❌   |   ❌    |
| tail       | completed   |  ✅   |  ✅   |   ✅    |
| tee        | completed   |  ✅   |  ✅   |   ✅    |
| test       | not started |  ❌   |  ❌   |   ❌    |
| time       | not started |  ❌   |  ❌   |   ❌    |
| timeout    | not started |  ❌   |  ❌   |   ❌    |
| touch      | in progress |  ✅   |  ✅   |   ✅    |
| tr         | not started |  ❌   |  ❌   |   ❌    |
| true       | completed   |  ✅   |  ✅   |   ✅    |
| truncate   | not started |  ❌   |  ❌   |   ❌    |
| tsort      | not started |  ❌   |  ❌   |   ❌    |
| tty        | not started |  ❌   |  ❌   |   ❌    |
| umount     | not started |  ❌   |  ❌   |   ❌    |
| uname      | completed   |  ✅   |  ✅   |   ✅    |
| unexpand   | not started |  ❌   |  ❌   |   ❌    |
| uniq       | not started |  ❌   |  ❌   |   ❌    |
| unlink     | in progress |  ✅   |  ✅   |   ✅    |
| uptime     | completed   |  ✅   |  ✅   |   ✅    |
| users      | not started |  ❌   |  ❌   |   ❌    |
| vdir       | in progress |  ✅   |  ✅   |   ✅    |
| wc         | completed   |  ✅   |  ✅   |   ✅    |
| which      | not started |  ❌   |  ❌   |   ❌    |
| who        | completed   |  ✅   |  ✅   |   ✅    |
| whoami     | completed   |  ✅   |  ✅   |   ✅    |
| yes        | completed   |  ✅   |  ✅   |   ✅    |


========================================================================

Some of these are implemented, may get to others.

From procps:

* free - Report the amounts of free and used memory in the system
* hugetop - Report hugepage usage of processes and the system as a whole
* kill - Send a signal to a process based on PID
* pgrep - List processes based on name or other attributes
* pkill - Send a signal to a process based on name or other attributes
* pmap - Report the memory map of a process
* ps - Report process information including PID and resource usage
* pwdx - Report the current working directory of a process
* slabtop - Display kernel slab cache information in real time
* snice - Renice a process
* sysctl - Read or write kernel parameters at run-time
* tload - Graphical representation of system load average
* top - Dynamic real-time view of running processes
* uptime - Display how long the system has been running
* vmstat - Report virtual memory statistics
* w - Report logged in users and what they are doing
* watch - Execute a program periodically, showing output fullscreen

From util-linux:

* bits(1) - convert bit masks or lists from/to various formats
* cal(1) - display a calendar
* chfn(1) - change your finger information
* choom(1) - display or adjust OOM-killer score
* chrt(1) - manipulate the real-time attributes of a process
* chsh(1) - change your login shell
* col(1) - filter reverse line feeds from input
* colcrt(1) - filter nroff output for CRT previewing
* colrm(1) - remove columns from a file
* column(1) - columnate lists
* copyfilerange(1) - copy range(s) from source to destination file
* coresched(1) - manage core scheduling cookies for tasks
* dmesg(1) - print or control the kernel ring buffer
* eject(1) - eject removable media
* enosys(1) - utility to make syscalls fail with ENOSYS
* exch(1) - atomically exchanges paths between two files
* fadvise(1) - utility to use the *posix_fadvise*(2) system call
* fallocate(1) - preallocate or deallocate space to a file
* fincore(1) - count pages of file contents in core
* flock(1) - manage locks from shell scripts
* getino(1) - print the unique inode number associated to a process file descriptor or namespace for a given PID
* getopt(1) - parse command options (enhanced)
* hardlink(1) - link multiple copies of a file
* hexdump(1) - display file contents in hexadecimal, decimal, octal, or ascii
* ionice(1) - set or get process I/O scheduling class and priority
* ipcmk(1) - make various IPC resources
* ipcrm(1) - remove certain IPC resources
* ipcs(1) - show information on IPC facilities
* irqtop(1) - display kernel interrupt information
* kill(1) - terminate a process
* last(1) - show a listing of last logged-in users
* lastb(1) - list logins on the system
* logger(1) - enter messages into the system log
* login(1) - begin session on the system
* look(1) - display lines beginning with a given string
* lsclocks(1) - display system clocks
* lscpu(1) - display information about the CPU architecture
* lsfd(1) - list file descriptors
* lsipc(1) - show information on IPC facilities currently employed in the system
* lsirq(1) - utility to display kernel interrupt information
* lslogins(1) - display information about known users in the system
* lsmem(1) - list the ranges of available memory with their online status
* mcookie(1) - generate magic cookies for xauth
* mesg(1) - display (or do not display) messages from other users
* more(1) - display the contents of a file in a terminal
* mountpoint(1) - see if a directory or file is a mountpoint
* namei(1) - follow a pathname until a terminal point is found
* nsenter(1) - run program in different namespaces
* pipesz(1) - set or examine pipe and FIFO buffer sizes
* prlimit(1) - get and set process resource limits
* rename(1) - rename files
* renice(1) - alter priority of running processes
* rev(1) - reverse lines characterwise
* runuser(1) - run a command with substitute user and group ID
* script(1) - make typescript of terminal session
* scriptlive(1) - re-run session typescripts, using timing information
* scriptreplay(1) - play back typescripts, using timing information
* setpgid(1) - run a program in a new process group
* setpriv(1) - run a program with different Linux privilege settings
* setsid(1) - run a program in a new session
* setterm(1) - set terminal attributes
* su(1) - run a command with substitute user and group ID
* taskset(1) - set or retrieve a process's CPU affinity
* terminator(1) - multiple GNOME terminals in one window
* uclampset(1) - manipulate the utilization clamping attributes of the system or a process
* ul(1) - do underlining
* unshare(1) - run program in new namespaces
* utmpdump(1) - dump UTMP and WTMP files in raw format
* uuidgen(1) - create a new UUID value
* uuidparse(1) - a utility to parse unique identifiers
* waitpid(1) - utility to wait for arbitrary processes
* wall(1) - write a message to all users
* whereis(1) - locate the binary, source, and manual page files for a command
* addpart(8) - tell the kernel about the existence of a partition
* agetty(8) - alternative Linux getty
* blkdiscard(8) - discard sectors on a device
* blkid(8) - locate/print block device attributes
* blkpr(8) - run a persistent-reservations command on a device
* blkzone(8) - run zone command on a device
* blockdev(8) - call block device ioctls from the command line
* cfdisk(8) - display or manipulate a disk partition table
* chcpu(8) - configure CPUs
* chmem(8) - configure memory
* ctrlaltdel(8) - set the function of the Ctrl-Alt-Del combination
* delpart(8) - tell the kernel to forget about a partition
* fdisk(8) - manipulate disk partition table
* findfs(8) - find a filesystem by label or UUID
* findmnt(8) - find a filesystem
* fsck(8) - check and repair a Linux filesystem
* fsck.cramfs(8) - fsck compressed ROM filesystem
* fsck.minix(8) - check consistency of Minix filesystem
* fsfreeze(8) - suspend access to a filesystem (Ext3/4, ReiserFS, JFS, XFS)
* fstrim(8) - discard unused blocks on a mounted filesystem
* hwclock(8) - time clocks utility
* i386(8) - change reported architecture in new program environment and/or set personality flags
* isosize(8) - output the length of an iso9660 filesystem
* lastlog2(8) - displays date of last login for all users or a specific one
* ldattach(8) - attach a line discipline to a serial line
* linux32(8) - change reported architecture in new program environment and/or set personality flags
* linux64(8) - change reported architecture in new program environment and/or set personality flags
* losetup(8) - set up and control loop devices
* lsblk(8) - list block devices
* lslocks(8) - list local system locks
* lsns(8) - list namespaces
* mkfs(8) - build a Linux filesystem
* mkfs.bfs(8) - make an SCO bfs filesystem
* mkfs.cramfs(8) - make compressed ROM filesystem
* mkfs.minix(8) - make a Minix filesystem
* mkswap(8) - set up a Linux swap area
* mount(8) - mount a filesystem
* nologin(8) - politely refuse a login
* pam_lastlog2(8) - PAM module to display date of last login
* partx(8) - tell the kernel about the presence and numbering of on-disk partitions
* pivot_root(8) - change the root filesystem
* readprofile(8) - read kernel profiling information
* resizepart(8) - tell the kernel about the new size of a partition
* rfkill(8) - tool for enabling and disabling wireless devices
* rtcwake(8) - enter a system sleep state until specified wakeup time
* setarch(8) - change reported architecture in new program environment and/or set personality flags
* sfdisk(8) - display or manipulate a disk partition table
* sulogin(8) - single-user login
* swaplabel(8) - print or change the label or UUID of a swap area
* swapoff(8) - enable/disable devices and files for paging and swapping
* swapon(8) - enable/disable devices and files for paging and swapping
* switch_root(8) - switch to another filesystem as the root of the mount tree
* umount(8) - unmount filesystems
* uname26(8) - change reported architecture in new program environment and/or set personality flags
* uuidd(8) - UUID generation daemon
* wdctl(8) - show hardware watchdog status
* wipefs(8) - wipe a signature from a device
* x86_64(8) - change reported architecture in new program environment and/or set personality flags
* zramctl(8) - set up and control zram devices

Copyright (C) 2014-2026 Darren Kirby <darren@dragonbyte.ca>
