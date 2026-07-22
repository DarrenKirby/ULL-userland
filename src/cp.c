/***************************************************************************
 *   cp.c - copy files to a new location                                   *
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

#if defined(__APPLE__) && defined(__MACH__)
#include <copyfile.h>
#endif
#if defined(__LINUX__)
#define _GNU_SOURCE
#define _FILE_OFFSET_BITS 64
#include <sys/types.h>
#endif
#include <getopt.h>
#include <fcntl.h>

#include "common.h"


static const char *APP_NAME = "cp";
static size_t PATH_MAX;

static struct {
    bool interactive;
    bool verbose;
    bool recursive;
    bool force;
} opts = { .interactive = false,
           .verbose     = false,
           .recursive   = false,
           .force       = false };

#define BUFF_SIZE 4096

static void show_help() {
    printf("Usage: %s [OPTION] file1 file2 [file1 dir1]\n\n\
Copy files to a new location\n\n\
    -h, --help\t\tdisplay this help\n\
    -i, --interactive\tprompt before overwriting existing files\n\
    -f, --force\tdo not prompt before overwriting existing files\n\
    -v, --verbose\tprint which files were copied\n\
    -V, --version\tdisplay version information\n\n\
Report bugs to <darren@dragonbyte.ca>\n", APP_NAME);
}

static int prompt(char *to) {
    printf("%s: %s exists. Overwrite ('y' or 'n')? ", APP_NAME, to);
    int response;
    do {
        response = getchar();
    }
    while (response == '\n');

    if (response == 'y' || response == 'Y') {
        return 1;
    }
    return 0;
}

static int copy_file(char *from, char *to) {
    int rv = 0;
    int from_fd = -1;
    int to_fd = -1;

    /* Open 'from' for reading. */
    if ((from_fd = open(from, O_RDONLY)) == -1) {
        fprintf(stderr, "%s: unable to open '%s': %s\n",
            APP_NAME, from, strerror(errno));
        return EXIT_FAILURE;
    }

    /* stat 'from' to get file size. */
    struct stat from_buf;
    if (fstat(from_fd, &from_buf) == -1) {
        fprintf(stderr, "%s: unable to stat '%s': %s\n",
            APP_NAME, from, strerror(errno));
        rv = EXIT_FAILURE;
        goto cleanup;
    }

    /* If --interactive, check if destination exists. */
    if (opts.interactive && access(to, F_OK) == 0) {
        if (!prompt(to)) {
            printf("%s: skipping '%s'\n", APP_NAME, to);
            rv = EXIT_SUCCESS;
            goto cleanup;
        }
    }

    /* Ensure 'from' is a regular file. */
    if ((from_buf.st_mode & S_IFMT) != S_IFREG) {
        fprintf(stderr, "%s: '%s' must be a regular file\n", APP_NAME, from);
        rv = EXIT_FAILURE;
        goto cleanup;
    }

    const size_t bytes_expected = from_buf.st_size;
    const mode_t perms = from_buf.st_mode;

    /* Open 'to' for writing. */
    if ((to_fd = open(to, O_WRONLY|O_CREAT|O_TRUNC, perms)) == -1) {
        fprintf(stderr, "%s: unable to open '%s': %s\n",
            APP_NAME, to, strerror(errno));
        rv = EXIT_FAILURE;
        goto cleanup;
    }
#if defined(__APPLE__) && defined(__MACH__)
    /* Stupid Apple doesn't have copy_file_range(). */
    const ssize_t bytes_copied = fcopyfile(from_fd, to_fd, nullptr, COPYFILE_DATA);
#else
    ssize_t bytes_copied = copy_file_range(from_fd, nullptr, to_fd, nullptr, bytes_expected, 0);
#endif
    if (bytes_copied == -1) {
        fprintf(stderr, "%s: copy of '%s' to '%s' failed: %s\n", APP_NAME, from, to, strerror(errno));
        rv = EXIT_FAILURE;
        goto cleanup;
    }
    if (bytes_expected < (size_t)bytes_copied) {
        fprintf(stderr, "%s: copy of '%s' to '%s' was truncated\n", APP_NAME, to, from);
        rv = EXIT_FAILURE;
        goto cleanup;
    }

    if (opts.verbose) {
        printf("'%s' copied to '%s'\n", from, to);
    }

cleanup:
    if (from_fd != -1) { close(from_fd); }
    if (to_fd !=  -1 ) { close(to_fd); }
    return rv;
}

int main(const int argc, char *argv[]) {
    PATH_MAX = get_path_max();

    const struct option long_opts[] = {
        {.name = "help",        .has_arg = 0, .flag = nullptr, .val = 'h'},
        {.name = "version",     .has_arg = 0, .flag = nullptr, .val = 'V'},
        {.name = "interactive", .has_arg = 0, .flag = nullptr, .val = 'i'},
        {.name = "verbose",     .has_arg = 0, .flag = nullptr, .val = 'v'},
        {.name = "recursive",   .has_arg = 0, .flag = nullptr, .val = 'r'},
        {.name = "force",       .has_arg = 0, .flag = nullptr, .val = 'f'},
        {.name = nullptr,       .has_arg = 0, .flag = nullptr, .val = 0}
    };

    int opt;
    while ((opt = getopt_long(argc, argv, "VhvirRf", long_opts, nullptr)) != -1) {
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
                /* --interactive and --force are mutually exclusive.
                 * The last flag passed will be the one that 'wins'. */
            case 'i':
                opts.interactive = true;
                opts.force = false;
                break;
            case 'f':
                opts.force = true;
                opts.interactive = false;
                break;
            case 'v':
                opts.verbose = true;
                break;
            case 'r':
            case 'R':
                opts.recursive = true;
                break;
            case '?':
            default:
                show_help();
                return EXIT_FAILURE;
        }
    }

    if (argc < 3) {
        fprintf(stderr, "%s: Missing destination operand\n\n", APP_NAME);
        show_help();
        return EXIT_FAILURE;
    }

    if (opts.recursive) {
        printf("Not implemented yet\n");
        return EXIT_FAILURE;
    }

    /* Check if last op is a dir. */
    struct stat op_last_buf;
    bool last_op_is_dir = true;

    errno = 0;
    if (stat(argv[argc - 1], &op_last_buf) == -1) {
        if (errno == ENOENT) {
            /* File does not exist, which is fine.
             * Just note it is not a directory. */
            last_op_is_dir = false;
        } else {
            /* Some other stat() error. */
            fprintf(stderr, "%s: stat of %s failed: %s\n", APP_NAME, argv[argc - 1], strerror(errno));
            return EXIT_FAILURE;
        }
    }

    if (last_op_is_dir && (op_last_buf.st_mode & S_IFMT) != S_IFDIR) {
        /* File exists, but is not a dir. This is
         * an error unless --force is used. */
        if (!opts.force) {
            fprintf(stderr, "%s: destination arg '%s' already exists\n", APP_NAME, argv[argc - 1]);
            return EXIT_FAILURE;
        }
        last_op_is_dir = false;
    }

    char destination[PATH_MAX];

    /* Exactly two operands, with no recurse.
     * op1 must be a regular file, op2 may be
     * a regular file or directory. */
    if (argc == 3) {
        if (last_op_is_dir) {
            snprintf(destination, PATH_MAX, "%s/%s", argv[argc - 1], argv[1]);
        } else {
            strncpy(destination, argv[argc - 1], PATH_MAX);
        }
        return copy_file(argv[1], destination);
    }

    /* More than two operands, with no recurse.
     * last op MUST be a directory, all others
     * MUST be regular files. */
    if (!last_op_is_dir) {
        fprintf(stderr, "%s: destination arg '%s' must be a directory\n", APP_NAME, argv[argc - 1]);
        return EXIT_FAILURE;
    }

    /* Iterate over all args except the last. */
    int rs = 0;
    while (optind < argc - 1) {
        snprintf(destination, PATH_MAX, "%s/%s", argv[argc - 1], argv[optind]);
        rs = copy_file(argv[optind++], destination);
    }

    return rs;
}
