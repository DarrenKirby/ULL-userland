/***************************************************************************
 *   base32 - encode/decode data and print to standard output              *
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
#include <stdlib.h>

#include "common.h"


static const char *APP_NAME = "base32";

static struct {
    bool decode;
    bool ignore;
    uint8_t wrap;

} opts = {
    .decode = false,
    .ignore = false,
    .wrap = 76 };

static int8_t decode_map[256];

static void init_decode_map()
{
    /* Set everything to -1 (garbage/invalid) by default. */
    for (int i = 0; i < 256; i++) {
        decode_map[i] = -1;
    }

    /* Map A-Z and a-z to 0-25. */
    for (int i = 0; i < 26; i++) {
        decode_map['A' + i] = i;
        decode_map['a' + i] = i;
    }

    /* Map 2-7 to 26-31. */
    for (int i = 0; i < 6; i++) {
        decode_map['2' + i] = i + 26;
    }

    /*  Map special characters. */
    decode_map['='] = -3;
    decode_map['\n'] = -2;
    decode_map['\r'] = -2;
}

static constexpr char digits[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ234567";

static void show_help()
{
    printf("Usage: %s [OPTION]...\n\n\
Options:\n\
    -h, --help\t\t display this help\n\
    -V, --version\t display version information\n\
    -d, --decode\t decode base32 encoded data\n\
    -i, --ignore-garbage\t ignore non-base32 characters\n\
    -r, --wrap=N\t wrap output at N characters\n\
Report bugs to <darren@dragonbyte.ca>\n", APP_NAME);
}

static FILE* open_infile(char *file)
{
    errno = 0;
    FILE *fp = fopen(file, "r");
    if (!fp) {
        fprintf(stderr, "%s: unable to open %s: %s", APP_NAME, file, strerror(errno));
        exit(EXIT_FAILURE);
    }
    return fp;
}

static void decode(char *name)
{
    FILE *fp;
    if (*name == 0x2D) {
        fp = stdin;
    } else {
        fp = open_infile(name);
    }

    int ch;
    uint8_t chars_read = 0;
    uint8_t pad_chars = 0;
    uint64_t val = 0;

    while ((ch = fgetc(fp)) != EOF) {
        const int8_t decoded = decode_map[ch];

        /* Newlines are always ignored. */
        if (decoded == -2) {
            continue;
        }

        /* Handle garbage. */
        if (decoded == -1) {
            if (opts.ignore) {
                continue;
            }
            fprintf(stderr, "%s: Invalid Base32 character '%c'\n", APP_NAME, ch);
            exit(EXIT_FAILURE);
        }

        /* Handle valid chars and Padding. */
        if (decoded == -3) {
            pad_chars++;
            val = (val << 5); /* Shift in 5 zero-bits to keep alignment. */
        } else {
            val = (val << 5) | (uint8_t)decoded;
        }

        chars_read++;

        /* Process the assembled 40-bit chunk. */
        if (chars_read == 8) {
            uint8_t out_bytes[5];
            out_bytes[0] = (val >> 32) & 0xFF;
            out_bytes[1] = (val >> 24) & 0xFF;
            out_bytes[2] = (val >> 16) & 0xFF;
            out_bytes[3] = (val >> 8)  & 0xFF;
            out_bytes[4] =  val        & 0xFF;

            /* Determine how many bytes to actually write based on padding count. */
            size_t bytes_to_write = 5;
            if (pad_chars == 6) bytes_to_write = 1;
            else if (pad_chars == 4) bytes_to_write = 2;
            else if (pad_chars == 3) bytes_to_write = 3;
            else if (pad_chars == 1) bytes_to_write = 4;

            fwrite(out_bytes, 1, bytes_to_write, stdout);

            /* Reset state for the next iteration. */
            chars_read = 0;
            pad_chars = 0;
            val = 0;
        }
    }
}

static void encode(char *name)
{
    FILE *fp;
    if (*name == 0x2D) {
        fp = stdin;
    } else {
        fp = open_infile(name);
    }

    char *out_buf = malloc(opts.wrap + 8);
    if (!out_buf) {
        fprintf(stderr, "%s: malloc failed!\n", APP_NAME);
        exit(EXIT_FAILURE);
    }

    uint8_t out_idx = 0;
    uint8_t in_buf[5];

    /* Read in 5-byte chunks, and parse them into eight
     * 5-bit integers. Then use the integers as indices
     * into the digit array to build the output string. */
    while (true) {
        const size_t read = fread(in_buf, 1, 5, fp);

        if (read == 0) {
            break;
        }

        if (read < 5) {
            for (int i = read; i < 5; i++) {
                in_buf[i] = 0;
            }
        }

        /* Pack the 5-bytes read into a single integer. */
        const uint64_t val = ((uint64_t)in_buf[0] << 32) |
                             ((uint64_t)in_buf[1] << 24) |
                             ((uint64_t)in_buf[2] << 16) |
                             ((uint64_t)in_buf[3] << 8)  |
                              (uint64_t)in_buf[4];

        /* Split into eight 5-bit integers. */
        out_buf[out_idx++] = digits[(val >> 35) & 0x1F];
        out_buf[out_idx++] = digits[(val >> 30) & 0x1F];
        out_buf[out_idx++] = digits[(val >> 25) & 0x1F];
        out_buf[out_idx++] = digits[(val >> 20) & 0x1F];
        out_buf[out_idx++] = digits[(val >> 15) & 0x1F];
        out_buf[out_idx++] = digits[(val >> 10) & 0x1F];
        out_buf[out_idx++] = digits[(val >>  5) & 0x1F];
        out_buf[out_idx++] = digits[val         & 0x1F];

        /* For short reads, determine the amount
         * of '=' padding, then backtrack and
         * overwrite the trailing 'A's. */
        if (read < 5) {
            uint8_t pad_chars = 0;
            if (read == 1) pad_chars = 6;
            else if (read == 2) pad_chars = 4;
            else if (read == 3) pad_chars = 3;
            else if (read == 4) pad_chars = 1;

            for (int i = 0; i < pad_chars; i++) {
                out_buf[out_idx - 1 - i] = '=';
            }
        }

        /* Print exactly opts.wrap chars to the console.
         * If there are extra chars from the previous
         * read then we copy them to the start of the
         * buffer, pad with '\0' and iterate again. */
        if (out_idx > opts.wrap) {
            const uint8_t ext = out_idx - opts.wrap;
            printf("%.*s\n", opts.wrap, out_buf);
            if (ext > 0) {
                memmove(out_buf, out_buf + opts.wrap, ext);
                memset(out_buf + ext, 0, opts.wrap + 8 - ext);
            }
            out_idx = ext;
        }
    }
    printf("%.*s\n", out_idx, out_buf);
}

int main(const int argc, char *argv[])
{
    const struct option long_opts[] = {
        {.name = "help",           .has_arg = no_argument,       .flag = nullptr, .val = 'h'},
        {.name = "version",        .has_arg = no_argument,       .flag = nullptr, .val = 'V'},
        {.name = "decode",         .has_arg = no_argument,       .flag = nullptr, .val = 'd'},
        {.name = "ignore-garbage", .has_arg = no_argument,       .flag = nullptr, .val = 'i'},
        {.name = "wrap",           .has_arg = required_argument, .flag = nullptr, .val = 'w'},
        {.name = nullptr,          .has_arg = no_argument,       .flag = nullptr, .val = 0}
    };

    int opt;
    while ((opt = getopt_long(argc, argv, "Vhdiw:", long_opts, NULL)) != -1) {
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
            case 'd':
                opts.decode = true;
                break;
            case 'i':
                opts.ignore = true;
                break;
            case 'w':
                opts.wrap = parse_numeric_arg(optarg, 1, 255, APP_NAME);
                break;
            default:
                show_help();
                return EXIT_FAILURE;
        }
    }

    if (argc == optind || strcmp(argv[optind], "-") == 0) {  /* no file arguments */
        if (opts.decode) {
            init_decode_map();
            decode("-");
        } else {
            encode("-");
        }
        return EXIT_SUCCESS;
    }

    while (optind < argc) {
        if (opts.decode) {
            init_decode_map();
            decode(argv[optind]);
            optind++;
        } else {
            encode(argv[optind]);
            optind++;
        }
    }

    return EXIT_SUCCESS;
}
