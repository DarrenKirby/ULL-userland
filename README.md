These are the ULL-userland core utilities.

These programs are one man's silly attempt to rewrite a UNIX userland
for use as the userland for ULL (Userland Linux), a bastard hybrid of the
Linux kernel, GLIBC, and a *BSDish userland. It remains to be seen if this
will ever happen, so for now, they are an exercise in C programming.

I intend to make these programs run on Linux, macOS, and *BSD.

See the [TODO.md](TODO.md) file for more details on programs I intend to write,
and status of 'in progress' programs, as well as notes on portability.

See the file INSTALL for compilation and installation instructions.

As it stands, these programs are different from the GNU versions in several
fundamental ways.  First of all, it is not the first, nor even an important
goal for these programs to be drop-in replacements.  Therefore, not all
functionality or command-line options/arguments are interfaced identically.
For example, I have tried to make the ULL utils have a consistent interface,
so I have adhered to standards such as `-h` and `--help` *always* display
a usage/option summary, and `-V` and `--version` *always* display version
information.  At times this convention clashes with the GNU version options.

For some of these programs, I do not intend to mimic all functionality of the
GNU versions for reasons of either personal interest, and/or technical ability.
Do not assume that because the GNU version does something, the ULL version
does as well.

========================================================================

Copyright © 2014-2025 Darren Kirby <bulliver@gmail.com>
