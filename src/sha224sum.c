/***************************************************************************
 *   sha224sum.c - compute and check sha224 message digest                 *
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

#include <stdint.h>
#include <getopt.h>

#include "sha2.h"
#include "common.h"

uint32_t *words;
uint8_t  *in_buf;

static const char *APP_NAME = "sha224sum";

static struct {
    bool check;
    bool bsd_style;
} opts = {
    .check = false,
    .bsd_style = false };

/* The eight 32-bit unsigned registers. */
uint32_t reg_h0 = 0xc1059ed8;
uint32_t reg_h1 = 0x367cd507;
uint32_t reg_h2 = 0x3070dd17;
uint32_t reg_h3 = 0xf70e5939;
uint32_t reg_h4 = 0xffc00b31;
uint32_t reg_h5 = 0x68581511;
uint32_t reg_h6 = 0x64f98fa7;
uint32_t reg_h7 = 0xbefa4fa4;

static void show_help()
{
    printf("Usage: %s [OPTION]...\n\n\
Options:\n\
    -h, --help\t\t display this help\n\
    -V, --version\t display version information\n\n\
Report bugs to <darren@dragonbyte.ca>\n", APP_NAME);
}

int main(const int argc, char *argv[]) {
    const struct option long_opts[] = {
        { .name = "help",      .has_arg = no_argument, .flag = nullptr, .val = 'h' },
        { .name = "version",   .has_arg = no_argument, .flag = nullptr, .val = 'V' },
        { .name = "check",     .has_arg = no_argument, .flag = nullptr, .val = 'c' },
        { .name = "bsd_style", .has_arg = no_argument, .flag = nullptr, .val = 'b' },
        { .name = nullptr,     .has_arg = no_argument, .flag = nullptr, .val = 0 }
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
            case 'c':
                opts.check = true;
                break;
            case 'b':
                opts.bsd_style = true;
                break;
            default:
                show_help();
                return EXIT_FAILURE;
        }
    }

    /* Allocate a read buffer and a word buffer. */
    in_buf = malloc(sizeof(uint8_t)  * 64);
    words  = malloc(sizeof(uint32_t) * 64);
    if (!in_buf || !words) {
        /* This is fatal no matter what. */
        fprintf(stderr, "%s: failed to allocate memory!\n", APP_NAME);
        return EXIT_FAILURE;
    }

    bool read_stdin = false;
    
    /* For stdin, we need to just read data until
     * we get an EOF, then determine the size, add
     * the padding etc etc. */
    if (argc == optind || strcmp(argv[optind], "-") == 0) {
        read_stdin = true;
    }

    /* Possibly multiple file arguments. Iterate
     * over them and print the digest on a separate line. */
    do {
        /* We need the message size in bits as an uint64_t. */
        uint64_t message_size = 0;

        /* Open the file. */
        FILE *fh;
        if (read_stdin) {
            fh = stdin;
        } else {
            fh = fopen(argv[optind], "r");
            if (!fh) {
                fprintf(stderr, "%s: unable to open %s: %s",
                    APP_NAME, argv[optind], strerror(errno));
            }
        }

        /* As long as we can read 64 byte chunks, process them. */
        size_t bytes_read;
        while ((bytes_read = fread(in_buf, 1, 64, fh)) == 64) {
            process_chunk_32(in_buf);
            message_size += bytes_read;
        }

        /* In bits.... */
        message_size = (message_size + bytes_read) * 8;

        /* No more full chunks. Pad and process the last chunk. */
        if (bytes_read >= 56) {
            /* Edge case where partial read is too large
             * to fit the padding and requires 2 chunks. */
            in_buf[bytes_read++] = 0x80;
            memset(in_buf + bytes_read, 0, 64 - bytes_read);
            process_chunk_32(in_buf);

            memset(in_buf, 0, 56);
        }
        else {
            in_buf[bytes_read++] = 0x80;
            memset(in_buf + bytes_read, 0, 56 - bytes_read);
        }

        /* Encode the 64-bit file size, big-endian. */
        in_buf[56] = (uint8_t)(message_size >> 56) & 0xFF;
        in_buf[57] = (uint8_t)(message_size >> 48) & 0xFF;
        in_buf[58] = (uint8_t)(message_size >> 40) & 0xFF;
        in_buf[59] = (uint8_t)(message_size >> 32) & 0xFF;
        in_buf[60] = (uint8_t)(message_size >> 24) & 0xFF;
        in_buf[61] = (uint8_t)(message_size >> 16) & 0xFF;
        in_buf[62] = (uint8_t)(message_size >> 8)  & 0xFF;
        in_buf[63] = (uint8_t)message_size         & 0xFF;

        /* Process the last chunk. */
        process_chunk_32(in_buf);

        /* Print the result. */
        printf("%08x%08x%08x%08x%08x%08x%08x", reg_h0, reg_h1, reg_h2, reg_h3,
                                               reg_h4, reg_h5, reg_h6);
        printf("  %s\n", read_stdin ? "-" : argv[optind]);

        if (read_stdin) goto exit;

        optind++;

        /* Reset the registers. */
        reg_h0 = 0xc1059ed8;
        reg_h1 = 0x367cd507;
        reg_h2 = 0x3070dd17;
        reg_h3 = 0xf70e5939;
        reg_h4 = 0xffc00b31;
        reg_h5 = 0x68581511;
        reg_h6 = 0x64f98fa7;
        reg_h7 = 0xbefa4fa4;

    } while (optind < argc);

exit:
    free(in_buf);
    free(words);
    return EXIT_SUCCESS;
}
