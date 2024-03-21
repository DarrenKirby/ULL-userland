### Current status of programs

It looks like I'm picking this up again after nine years.
Now as then, this is just an exercise to teach myself C.
My Linux and FreeBSD machine is in storage so I only have
MacBook Pro to write and test code on presently. To wit:

2.2 GHz 6-Core Intel Core i7 running Ventura 13.0
gcc-13 (Homebrew GCC 13.2.0) 13.2.0
Apple clang version 14.0.3 (clang-1403.0.22.14.1)
Target: x86_64-apple-darwin22.1.0

I'll putter around as I find time to do so. In the meantime,
I've decided to keep my original note from 2014 for historic
and sentimental reasons:

=======================================================================

Note: Just because the status is 'completed' doesn't mean I won't
still hack on it. Just means the core functionality that I intend
to implement is there. Over time I will add more error-checking or
refactoring, as well as adding portability fixes.

On portability: these programs are tested on three systems:
Gentoo Linux       (GCC 4.7.3, GLIBC 2.19)
OS X Mavericks     (LLVM 5.1, CLANG 503.0.40,)
FreeBSD 10-Release (CLANGBSD 3.3, )

...but developed on the linux machine for the most part. Am working
on portablility but a lot of the apps are flaky or don't even compile
on OS X and FeeBSD. See notes below...
========================================================================

**at**        :: not started
**base64**    :: not started
**basename**  :: completed    # Linux: compiles, works | OS X: compiles, works | FreeBSD: compiles, works
**cal**       :: not started
**cat**       :: completed    # Linux: compiles, works | OS X: compiles, works | FreeBSD: compiles, works
**cd**        :: completed    # Linux: compiles | OS X: compiles | FreeBSD: compiles (But useless on all: read cd.c source)
**chgrp**     :: completed    # Linux ?? | OS X: compiles, works | FreeBSD ??
**chmod**     :: not started
**chown**     :: completed    # Linux ?? | OS X: compiles, works | FreeBSD ??
**chpass**    :: not started
**chroot**    :: not started
**cksum**     :: not started
**comm**      :: not started
**cmp**       :: not started
**cp**        :: in progress
**csplit**    :: not started
**cut**       :: not started
**date**      :: not started
**dd**        :: not started
**df**        :: in progress
**dir**       :: not started
**dircolors** :: not started
**dirname**   :: in progress  # Linux ?? | OS X: compiles, works | FreeBSD ??
**dmesg**     :: not started
**domainname**:: not started
**du**        :: not started
**env**       :: in progress  # Works on Linux, and OS X
**expand**    :: not started
**expr**      :: not started
**factor**    :: not started
**false**     :: completed    # Linux: compiles, works | OS X: compiles, works | FreeBSD: compiles, works
**fmt**       :: not started
**fold**      :: not started
**free**      :: in progress  # Linux: compiles, works | OS X: compiles, works | FreeBSD: compiles, ??
**fuser**     :: not started
**head**      :: not started
**hostid**    :: not started
**hostname**  :: not started
**id**        :: not started
**ifconfig**  :: not started
**install**   :: not started
**join**      :: not started
**kill**      :: not started
**link**      :: in progress  # Linux ?? | OS X: compiles, works | FreeBSD ??
**ln**        :: in progress  # Linux ?? | OS X: compiles, works | FreeBSD ??
**logger**    :: not started
**logname**   :: not started
**ls**        :: in progress  # Linux ?? | OS X: compiles, works | FreeBSD ??
**md5sum**    :: not started
**mesg**      :: not started
**mkdir**     :: completed    # Linux: compiles, works | OS X: compiles, works | FreeBSD: compiles, works
**mkfifo**    :: not started
**mknod**     :: not started
**mount**     :: not started
**mv**        :: completed    # Linux: compiles, works | OS X: compiles, works | FreeBSD: compiles, works
**nice**      :: not started
**nl**        :: not started
**nohup**     :: not started
**od**        :: in progress  # flaky!!! Fix endianess
**paste**     :: not started
**pathchk**   :: not started
**ping**      :: not started
**pinky**     :: not started
**pr**        :: not started
**ps**        :: not started
**printenv**  :: completed    # Linux: compiles, works | OS X: compiles, works | FreeBSD: compiles, works
**printf**    :: not started
**ptx**       :: not started
**pwd**       :: completed    # Linux: compiles, works | OS X: compiles, works | FreeBSD: compiles, works
**readlink**  :: not started
**renice**    :: not started
**rm**        :: completed    # Linux: compiles, works | OS X: compiles, works | FreeBSD: compiles, works
**rmdir**     :: completed    # Linux: compiles, works | OS X: compiles, works | FreeBSD: compiles, works
**route**     :: not started
**seq**       :: not started
**sha1sum**   :: not started
**sha224sum** :: not started
**sha256sum** :: not started
**sha384sum** :: not started
**sha512sum** :: not started
**shred**     :: not started
**shuf**      :: not started
**sleep**     :: completed    # Linux: compiles, works | OS X: compiles, works | FreeBSD: compiles, works
**sort**      :: not started
**split**     :: not started
**stat**      :: completed    # Linux: compiles, works | OS X: compiles, works | FreeBSD: compiles, works
**stty**      :: not started
**sum**       :: not started
**sync**      :: completed    # Linux: compiles, works | OS X: compiles, works | FreeBSD: compiles, works
**tac**       :: not started
**tail**      :: not started
**talk**      :: not started
**tee**       :: not started
**telnet**    :: not started
**time**      :: not started
**touch**     :: completed    #
**tput**      :: not started
**tr**        :: not started
**true**      :: completed    # Linux: compiles, works | OS X: compiles, works | FreeBSD: compiles, works
**tsort**     :: not started
**tty**       :: not started
**umount**    :: not started
**uname**     :: completed    # Linux: compiles, works | OS X: compiles, works | FreeBSD: compiles, segfaults
**unexpand**  :: not started
**uniq**      :: not started
**unlink**    :: in progress  # Linux ?? | OS X: compiles, works | FreeBSD: ??
**uptime**    :: completed    # Linux: compiles, works | OS X: compiles, works | FreeBSD: untested
**users**     :: not started
**vdir**      :: in progress  # Mostly working including colour output
**wc**        :: completed    # Linux: compiles, works | OS X: compiles, works | FreeBSD: compiles, works
**which**     :: not started
**who**       :: not started
**whois**     :: not started
**whoami**    :: completed    # Linux: compiles, works | OS X: compiles, works | FreeBSD: compiles, works
**write**     :: not started
**yes**       :: completed    # Linux: compiles, works | OS X: compiles, works | FreeBSD: compiles, works

========================================================================

Copyright (C) 2014-2024 Darren Kirby <bulliver@gmail.com>
