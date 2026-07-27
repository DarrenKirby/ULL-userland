/***************************************************************************
 *   base64 - encode/decode data and print to standard output              *
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


static const char *APP_NAME = "base64";

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

    /* Map A-Z to 0-25. */
    for (int i = 0; i < 26; i++) {
        decode_map['A' + i] = i;
    }

    /* Map a-z to 26-51. */
    for (int i = 0; i < 26; i++) {
        decode_map['a' + i] = i + 26;
    }

    /* Map 0-9 to 52-61. */
    for (int i = 0; i < 10; i++) {
        decode_map['0' + i] = i + 52;
    }

    /* Map '+' and '/' to 62 and 63. */
    decode_map['+'] = 62;
    decode_map['/'] = 63;

    /*  Map special characters. */
    decode_map['=']  = -3;
    decode_map['\n'] = -2;
    decode_map['\r'] = -2;
}

static constexpr char digits[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

static void show_help()
{
    printf("Usage: %s [OPTION]...\n\n\
Options:\n\
    -h, --help\t\t display this help\n\
    -V, --version\t display version information\n\
    -d, --decode\t decode base64 encoded data\n\
    -i, --ignore-garbage\t ignore non-base64 characters\n\
    -w, --wrap=N\t wrap output at N characters. Use '0' for no wrapping\n\
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
    uint32_t val = 0;

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
            fprintf(stderr, "%s: Invalid Base64 character: '%c'\n", APP_NAME, ch);
            exit(EXIT_FAILURE);
        }

        /* Handle valid chars and Padding. */
        if (decoded == -3) {
            pad_chars++;
            val = val << 6; /* Shift in 6 zero-bits to keep alignment. */
        } else {
            val = (val << 6) | (uint8_t)decoded;
        }

        chars_read++;

        /* Process the assembled 24-bit chunk. */
        if (chars_read == 4) {
            uint8_t out_bytes[3];
            out_bytes[0] = (val >> 16) & 0xFF;
            out_bytes[1] = (val >> 8)  & 0xFF;
            out_bytes[2] =  val        & 0xFF;

            /* Determine how many bytes to actually write based on padding count. */
            size_t bytes_to_write =3;
            if (pad_chars == 1) bytes_to_write = 2;
            else if (pad_chars == 2) bytes_to_write = 1;

            fwrite(out_bytes, 1, bytes_to_write, stdout);

            /* Reset state for the next iteration. */
            chars_read = 0;
            pad_chars = 0;
            val = 0;
        }
    }

    if (chars_read > 0) {
        fprintf(stderr, "%s: warning: truncated message encountered!\n", APP_NAME);
    }

    if (fp != stdin) {
        fclose(fp);
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

    char *out_buf = malloc(opts.wrap + 4);
    if (!out_buf) {
        fprintf(stderr, "%s: malloc failed!\n", APP_NAME);
        exit(EXIT_FAILURE);
    }

    uint8_t out_idx = 0;
    uint8_t in_buf[3];

    /* Read in 3-byte chunks, and parse them into four
     * 6-bit integers. Then use the integers as indices
     * into the digit array to build the output string. */
    while (true) {
        const size_t read = fread(in_buf, 1, 3, fp);

        if (read == 0) {
            break;
        }

        if (read < 3) {
            for (int i = read; i < 3; i++) {
                in_buf[i] = 0;
            }
        }

        /* Pack the 3-bytes read into a single integer. */
        const uint32_t val = ((uint32_t)in_buf[0] << 16) |
                             ((uint32_t)in_buf[1] <<  8) |
                              (uint32_t)in_buf[2];

        /* Split into four 6-bit integers. */
        out_buf[out_idx++] = digits[(val >> 18) & 0x3F];
        out_buf[out_idx++] = digits[(val >> 12) & 0x3F];
        out_buf[out_idx++] = digits[(val >>  6) & 0x3F];
        out_buf[out_idx++] = digits[val         & 0x3F];

        /* For short reads, determine the amount
         * of '=' padding, then backtrack and
         * overwrite the trailing 'A's. */
        if (read < 3) {
            uint8_t pad_chars = 0;
            if (read == 1) pad_chars = 2;
            else if (read == 2) pad_chars = 1;

            for (int i = 0; i < pad_chars; i++) {
                out_buf[out_idx - 1 - i] = '=';
            }
        }

        /* Do not print newlines. */
        if (opts.wrap == 0) {
            printf("%.*s", out_idx, out_buf);
            out_idx = 0;
        } else {
            /* Print exactly opts.wrap chars and a newline.
             * If there are extra chars from the previous
             * read then we copy them to the start of the
             * buffer, pad with '\0' and iterate again. */
            while (out_idx > opts.wrap) {
                const uint8_t ext = out_idx - opts.wrap;
                printf("%.*s\n", opts.wrap, out_buf);
                if (ext > 0) {
                    memmove(out_buf, out_buf + opts.wrap, ext);
                    memset(out_buf + ext, 0, opts.wrap + 4 - ext);
                }
                out_idx = ext;
            }
        }
    }
    printf("%.*s\n", out_idx, out_buf);

    if (fp != stdin) {
        fclose(fp);
    }
    free(out_buf);
}

int main(const int argc, char *argv[])
{
    const struct option long_opts[] = {
        { .name = "help",           .has_arg = no_argument,       .flag = nullptr, .val = 'h' },
        { .name = "version",        .has_arg = no_argument,       .flag = nullptr, .val = 'V' },
        { .name = "decode",         .has_arg = no_argument,       .flag = nullptr, .val = 'd' },
        { .name = "ignore-garbage", .has_arg = no_argument,       .flag = nullptr, .val = 'i' },
        { .name = "wrap",           .has_arg = required_argument, .flag = nullptr, .val = 'w' },
        { .name = nullptr,          .has_arg = no_argument,       .flag = nullptr, .val = 0 }
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
                opts.wrap = parse_numeric_arg(optarg, 0, 255, APP_NAME);
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
            decode(argv[optind++]);
        } else {
            encode(argv[optind++]);
        }
    }

    return EXIT_SUCCESS;
}
