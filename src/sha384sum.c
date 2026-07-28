/***************************************************************************
 *   sha384sum.c - compute and check sha384 message digest                 *
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
#include <inttypes.h>

#include "sha2.h"
#include "common.h"

uint64_t *l_words;
uint8_t  *in_buf;

static const char *APP_NAME = "sha384sum";

static struct {
    bool check;
    bool bsd_style;
} opts = {
    .check = false,
    .bsd_style = false };

/* The eight 64-bit unsigned registers. */
uint64_t reg64_h0 = 0xcbbb9d5dc1059ed8;
uint64_t reg64_h1 = 0x629a292a367cd507;
uint64_t reg64_h2 = 0x9159015a3070dd17;
uint64_t reg64_h3 = 0x152fecd8f70e5939;
uint64_t reg64_h4 = 0x67332667ffc00b31;
uint64_t reg64_h5 = 0x8eb44a8768581511;
uint64_t reg64_h6 = 0xdb0c2e0d64f98fa7;
uint64_t reg64_h7 = 0x47b5481dbefa4fa4;

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
    in_buf  = malloc(sizeof(uint8_t)  * 128);
    l_words = malloc(sizeof(uint64_t) * 64);
    if (!in_buf || !l_words) {
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

        /* As long as we can read 128 byte chunks, process them. */
        size_t bytes_read;
        while ((bytes_read = fread(in_buf, 1, 128, fh)) == 128) {
            process_chunk_64(in_buf);
            message_size += bytes_read;
        }

        /* In bits.... */
        message_size = (message_size + bytes_read) * 8;

        /* No more full chunks. Pad and process the last chunk. */
        if (bytes_read >= 112) {
            /* Edge case where partial read is too large
             * to fit the padding and requires 2 chunks. */
            in_buf[bytes_read++] = 0x80;
            memset(in_buf + bytes_read, 0, 128 - bytes_read);
            process_chunk_64(in_buf);

            memset(in_buf, 0, 112);
        }
        else {
            in_buf[bytes_read++] = 0x80;
            memset(in_buf + bytes_read, 0, 112 - bytes_read);
        }

        /* Encode the 128-bit file size, big-endian. */

        /* Zero out the upper 64 bits (bytes 112 to 119) */
        memset(in_buf + 112, 0, 8);

        /* Encode the 64-bit message_size into the lower 64 bits (bytes 120 to 127) */
        in_buf[120] = (uint8_t)(message_size >> 56) & 0xFF;
        in_buf[121] = (uint8_t)(message_size >> 48) & 0xFF;
        in_buf[122] = (uint8_t)(message_size >> 40) & 0xFF;
        in_buf[123] = (uint8_t)(message_size >> 32) & 0xFF;
        in_buf[124] = (uint8_t)(message_size >> 24) & 0xFF;
        in_buf[125] = (uint8_t)(message_size >> 16) & 0xFF;
        in_buf[126] = (uint8_t)(message_size >> 8)  & 0xFF;
        in_buf[127] = (uint8_t)message_size         & 0xFF;

        /* Process the last chunk. */
        process_chunk_64(in_buf);

        /* Print the digest. */
        printf("%016" PRIx64 "%016" PRIx64 "%016" PRIx64 "%016" PRIx64
               "%016" PRIx64 "%016" PRIx64,
                reg64_h0, reg64_h1, reg64_h2, reg64_h3,
                reg64_h4, reg64_h5);
        printf("  %s\n", read_stdin ? "-" : argv[optind]);

        if (read_stdin) goto exit;

        optind++;

        /* Reset the registers. */
         reg64_h0 = 0xcbbb9d5dc1059ed8;
         reg64_h1 = 0x629a292a367cd507;
         reg64_h2 = 0x9159015a3070dd17;
         reg64_h3 = 0x152fecd8f70e5939;
         reg64_h4 = 0x67332667ffc00b31;
         reg64_h5 = 0x8eb44a8768581511;
         reg64_h6 = 0xdb0c2e0d64f98fa7;
         reg64_h7 = 0x47b5481dbefa4fa4;

    } while (optind < argc);

exit:
    free(in_buf);
    free(words);
    return EXIT_SUCCESS;
}
