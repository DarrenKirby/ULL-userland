/***************************************************************************
 *   md5sum.c - compute and check MD5 message digest                       *
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

#include "common.h"

#define INT_BITS 32


static const char *APP_NAME = "md5sum";

static struct {
    bool check;
    bool bsd_style;
} opts = {
    .check = false,
    .bsd_style = false };

/* The per-round shift amounts. */
static uint8_t shift_n[] = { 7, 12, 17, 22, 7, 12, 17, 22, 7, 12, 17, 22, 7, 12, 17, 22,
                             5,  9, 14, 20, 5,  9, 14, 20, 5,  9, 14, 20, 5,  9, 14, 20,
                             4, 11, 16, 23, 4, 11, 16, 23, 4, 11, 16, 23, 4, 11, 16, 23,
                             6, 10, 15, 21, 6, 10, 15, 21, 6, 10, 15, 21, 6, 10, 15, 21 };

/* A 64-element array k[1 ... 64] constructed from the sine function.
 * Let k[i] denote the i-th element of the table, which is equal to the
 * integer part of 4294967296 times abs(sin(i)), where i is in radians. */
static constexpr uint32_t k[] = { 0xd76aa478, 0xe8c7b756, 0x242070db, 0xc1bdceee,
                                  0xf57c0faf, 0x4787c62a, 0xa8304613, 0xfd469501,
                                  0x698098d8, 0x8b44f7af, 0xffff5bb1, 0x895cd7be,
                                  0x6b901122, 0xfd987193, 0xa679438e, 0x49b40821,
                                  0xf61e2562, 0xc040b340, 0x265e5a51, 0xe9b6c7aa,
                                  0xd62f105d, 0x02441453, 0xd8a1e681, 0xe7d3fbc8,
                                  0x21e1cde6, 0xc33707d6, 0xf4d50d87, 0x455a14ed,
                                  0xa9e3e905, 0xfcefa3f8, 0x676f02d9, 0x8d2a4c8a,
                                  0xfffa3942, 0x8771f681, 0x6d9d6122, 0xfde5380c,
                                  0xa4beea44, 0x4bdecfa9, 0xf6bb4b60, 0xbebfbc70,
                                  0x289b7ec6, 0xeaa127fa, 0xd4ef3085, 0x04881d05,
                                  0xd9d4d039, 0xe6db99e5, 0x1fa27cf8, 0xc4ac5665,
                                  0xf4292244, 0x432aff97, 0xab9423a7, 0xfc93a039,
                                  0x655b59c3, 0x8f0ccc92, 0xffeff47d, 0x85845dd1,
                                  0x6fa87e4f, 0xfe2ce6e0, 0xa3014314, 0x4e0811a1,
                                  0xf7537e82, 0xbd3af235, 0x2ad7d2bb, 0xeb86d391 };

/* The four 32-bit unsigned registers. */
static uint32_t reg_a = 0x67452301;
static uint32_t reg_b = 0xefcdab89;
static uint32_t reg_c = 0x98badcfe;
static uint32_t reg_d = 0x10325476;

static uint32_t *words;
static uint8_t  *in_buf;

static void show_help()
{
    printf("Usage: %s [OPTION]...\n\n\
Options:\n\
    -h, --help\t\t display this help\n\
    -V, --version\t display version information\n\n\
Report bugs to <darren@dragonbyte.ca>\n", APP_NAME);
}

/* Left-rotate n by d bits. */
static uint32_t left_rotate(const uint32_t n, const uint8_t d)
{
    return (n << d) | (n >> (INT_BITS - d));
}

/* Encode the chunk as 16 4-byte little-endian words. */
static void encode_words(const uint8_t *chunk) {
    for (int j = 0; j < 16; j++) {
        words[j] = ((uint32_t)chunk[4*j + 3] << 24) |
                   ((uint32_t)chunk[4*j + 2] << 16) |
                   ((uint32_t)chunk[4*j + 1] <<  8) |
                    (uint32_t)chunk[4*j];
    }
}

static void process_chunk(const uint8_t *chunk) {
    uint32_t tmp_A = reg_a;
    uint32_t tmp_B = reg_b;
    uint32_t tmp_C = reg_c;
    uint32_t tmp_D = reg_d;

    encode_words(chunk);

    for (int i = 0; i < 64; i++) {
        uint32_t f;
        uint32_t g;

        if (i < 16) {
            f = (tmp_B & tmp_C) | ((~tmp_B) & tmp_D);
            g = i;
        } else if (i < 32) {
            f = (tmp_D & tmp_B) | ((~tmp_D) & tmp_C);
            g = (5 * i + 1) % 16;
        } else if (i < 48) {
            f = tmp_B ^ tmp_C ^ tmp_D;
            g = (3 * i + 5) % 16;
        } else {
            f = tmp_C ^ (tmp_B | (~ tmp_D));
            g = (7 * i) % 16;
        }

        f = f + tmp_A + k[i] + words[g];
        tmp_A = tmp_D;
        tmp_D = tmp_C;
        tmp_C = tmp_B;
        tmp_B = tmp_B + left_rotate(f, shift_n[i]);
    }

    reg_a += tmp_A;
    reg_b += tmp_B;
    reg_c += tmp_C;
    reg_d += tmp_D;
}

static void print_register(const uint32_t r)
{
    for (int i = 0; i < 4; i++)
        printf("%02x", (r >> (8 * i)) & 0xff);
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
    words  = malloc(sizeof(uint32_t) * 16);
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
            process_chunk(in_buf);
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
            process_chunk(in_buf);

            memset(in_buf, 0, 56);
        }
        else {
            in_buf[bytes_read++] = 0x80;
            memset(in_buf + bytes_read, 0, 56 - bytes_read);
        }

        /* Encode the 64-bit file size, little-endian. */
        in_buf[56] = (uint8_t)message_size         & 0xFF;
        in_buf[57] = (uint8_t)(message_size >> 8)  & 0xFF;
        in_buf[58] = (uint8_t)(message_size >> 16) & 0xFF;
        in_buf[59] = (uint8_t)(message_size >> 24) & 0xFF;
        in_buf[60] = (uint8_t)(message_size >> 32) & 0xFF;
        in_buf[61] = (uint8_t)(message_size >> 40) & 0xFF;
        in_buf[62] = (uint8_t)(message_size >> 48) & 0xFF;
        in_buf[63] = (uint8_t)(message_size >> 56) & 0xFF;

        /* Process the last chunk. */
        process_chunk(in_buf);

        /* Print the result. */
        print_register(reg_a);
        print_register(reg_b);
        print_register(reg_c);
        print_register(reg_d);
        printf("  %s\n", read_stdin ? "-" : argv[optind]);

        if (read_stdin) goto exit;

        optind++;

        /* Reset the registers. */
        reg_a = 0x67452301;
        reg_b = 0xefcdab89;
        reg_c = 0x98badcfe;
        reg_d = 0x10325476;

    } while (optind < argc);

exit:
    free(in_buf);
    free(words);
    return EXIT_SUCCESS;
}
