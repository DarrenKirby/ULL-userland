/***************************************************************************
 *   od - dump files in hexidecimal and other formats                      *
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

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <inttypes.h>
#include <unistd.h>
#include <string.h>
#include <getopt.h>
#include <locale.h>
#include <limits.h>
#include <sys/stat.h>


#include "common.h"

/* Constants for box-drawing, and others. */
#define WELL_WIDTH 12
#define MID_DOT  0x00B7

/* Determine machine endianess for default output. */
#ifndef __BYTE_ORDER__
static bool little_endian = true;
#else
static bool little_endian = (__BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__);
#endif


static const char *APP_NAME = "od";

/* Static lookup tables for formatting hex and oct strings. */
static constexpr char hex_chars[] = "0123456789abcdef";
static constexpr char oct_chars[] = "01234567";

/* This is an arbitrary constant that sets the upper
 * read limit for piped input. */
#define MAX_READ_BYTES SIZE_MAX

/* A buffer large enough for the widest format:
 * (BIN: 9 chars * 255 bytes max = 2295 + 32B padding). */
#define MAX_LINE_BUF_LEN ((255 * 9) + 32)

/* 255 * 2 = 510 bytes + 2 more:
 * (space + newline. */
#define ASCII_BUF_SIZE ((255 * 2) + 2)

/* L1/L2 cache friendly read-buffer size. */
#define CHUNK_SIZE 8192

typedef enum : int8_t {
    F_HEX,
    F_OCT,
    F_UNSIGNED,
    F_SIGNED,
} format_t;

typedef enum : int8_t {
    O_BYTE,
    O_HALF_WORD,
    O_WORD
} output_t;

/* Default format: hex */
static format_t format = F_HEX;
/* Default output: half words. */
static output_t output = O_HALF_WORD;
/* Default line_width: 16 */
static uint8_t line_width = 16;
/* Default is to read all bytes. This value will be filled by
 * call to stat() if --read-size is not used. */
static size_t read_size = 0;
/* This does not change per run, so cache it. */
static int32_t bin_width;
/* Print ascii dump? */
static bool ascii = false;


static void show_help()
{
    printf("Usage: %s [OPTION(s)] [FILE]\n\n\
Options:\n\
    Output radix options:\n\
        -x, --hex\t\t hexadecimal\n\
        -o, --oct\t\t octal\n\
        -S, --signed\t\t signed decimal\n\
        -d, --unsigned\t\t unsigned decimal\n\n\
    Output byte grouping options:\n\
        -b  --byte\t\t output single byte groupings\n\
        -H, --half-word\t\t output 2 byte groupings\n\
        -W, --word\t\t output 4 byte groupings\n\n\
    Output byte-order options:\n\
        -L, --little-endian\t output in little-endian\n\
        -B, --big-endian\t output in big-endian\n\n\
    General options:\n\
        -a, --ascii\t\t print ascii dump\n\
        -n, --no-elide\t\t don't elide lines of NULL bytes\n\
        -l, --line-width=n\t print n bytes per line\n\
        -s, --skip-bytes=n\t start output at offset n\n\
        -r, --read-bytes=n\t read only n bytes\n\
        -h, --help\t\t display this help\n\
        -V, --version\t\t display version information\n\n\
Report bugs to <darren@dragonbyte.ca>\n", APP_NAME);
}

static size_t get_file_size(const int fd)
{
    struct stat buf;
    if (fstat(fd, &buf) == -1) {
        fprintf(stderr, "stat failed: %s\n",
            strerror(errno));
        exit(EXIT_FAILURE);
    }
    if (!S_ISREG(buf.st_mode)) {
        /* Input was either piped or no input was supplied.
         * just return an arbitrary large number. */
        return MAX_READ_BYTES;
    }
    return buf.st_size;
}

/* Calculate the width of the binary section based on output
 * format and line_width. */
static int32_t get_bin_width(void)
{
    int n_groups;
    switch (output) {
        /* Half word. */
        case O_HALF_WORD:
            n_groups = line_width / 2;
            switch (format) {
                case F_UNSIGNED:
                    /* DEC: 5 chars + 1 space for each group + 1 space left side. */
                    return n_groups * 6 + 1;
                case F_OCT:
                case F_SIGNED:
                    /* Signed DEC and OCT: 6 chars + 1 space for each group + 1 space on left side. */
                    return n_groups * 7 + 1;
                default:
                    /* HEX: 4 chars + 1 space for each group + 1 space on left side. */
                    return n_groups * 5 + 1;
            }
            /* Full word. */
        case O_WORD:
            n_groups = line_width / 4;
            switch (format) {
                case F_UNSIGNED:
                    /* DEC: 10 chars + 1 space for each group + 1 space on left side. */
                    return n_groups * 11 + 1;
                case F_SIGNED:
                case F_OCT:
                    /* Signed DEC and OCT: 11 chars + 1 space for each group + 1 space on left side. */
                    return n_groups * 12 + 1;
                default:
                    /* HEX: 8 chars + 1 space for each + 1 space on left side. */
                    return n_groups * 9 + 1;
            }
            /* Single byte. */
        default:
            switch (format) {
                case F_UNSIGNED:
                case F_OCT:
                    /* OCT and DEC: 3 chars + 1 space for each + 1 space on left side. */
                    return line_width * 4 + 1;
                case F_SIGNED:
                    /* Signed DEC: 4 chars + 1 space for each, plus 1 space on left side. */
                    return line_width * 5 + 1;
                default:
                    /* HEX: 2 chars + 1 space for each + 1 space on left side. */
                    return line_width * 3 + 1;
            }
    }
}

/* Write the offset well section of output. Returns a boolean
 * indicating if we have printed the last data line. */
static bool write_well(const uint64_t offset, const size_t bytes_read)
{
    switch (format) {
    case F_OCT:
        if (bytes_read == 0) {
            printf(" 0o%08" PRIo64 "  ", offset);
            return true;
        }
        printf(" 0o%08" PRIo64 "  ", offset);
        return false;
    case F_UNSIGNED:
    case F_SIGNED:
        if (bytes_read == 0) {
            printf(" 0d%08" PRIu64 "  ", offset);
            return true;
        }
        printf(" 0d%08" PRIu64 "  ", offset);
        return false;
    default:
        if (bytes_read == 0) {
            printf(" 0x%08" PRIx64 "  ", offset);
            return true;
        }
        printf(" 0x%08" PRIx64 "  ", offset);
        return false;
    }
}

/* Build a half-word from 2 bytes. */
static uint16_t parse_half_word(const uint8_t a, const uint8_t b)
{
    uint16_t half_word;
    if (little_endian) {
        half_word = (uint16_t)b << 8 | (uint16_t)a;
    } else {
        half_word = (uint16_t)a << 8 | (uint16_t)b;
    }
    return half_word;
}

/* Zero-pad incomplete half-word groupings. */
static uint16_t load_half_word(const uint8_t *buf, const size_t remaining)
{
    const uint8_t a = remaining > 0 ? buf[0] : 0;
    const uint8_t b = remaining > 1 ? buf[1] : 0;

    return parse_half_word(a, b);
}

/* Build a word from 4 bytes. */
static uint32_t parse_word(const uint8_t a, const uint8_t b, const uint8_t c, const uint8_t d)
{
    uint32_t word;
    if (little_endian) {
        word = (uint32_t)d << 24 | (uint32_t)c << 16 | (uint32_t)b << 8 | (uint32_t)a;
    } else {
        word = (uint32_t)a << 24 | (uint32_t)b << 16 | (uint32_t)c << 8 | (uint32_t)d;
    }
    return word;
}

/* Zero-pad incomplete word groupings. */
static uint32_t load_word(const uint8_t *buf, size_t remaining)
{
    const uint8_t a = remaining > 0 ? buf[0] : 0;
    const uint8_t b = remaining > 1 ? buf[1] : 0;
    const uint8_t c = remaining > 2 ? buf[2] : 0;
    const uint8_t d = remaining > 3 ? buf[3] : 0;

    return parse_word(a, b, c, d);
}

static size_t write_hex_dump(char *line_buf, const uint8_t *buffer, const size_t bytes_read)
{
    size_t pos = 0;

    if (output == O_BYTE) {
        for (size_t i = 0; i < bytes_read; i++) {
            line_buf[pos++] = hex_chars[(buffer[i] >> 4) & 0x0F];
            line_buf[pos++] = hex_chars[buffer[i] & 0x0F];
            line_buf[pos++] = ' ';
        }
        return pos;
    }

    if (output == O_HALF_WORD) {
        for (size_t i = 0; i < bytes_read; i+=2) {
            const uint16_t half_word = load_half_word(&buffer[i], bytes_read - i);

            line_buf[pos++] = hex_chars[(half_word >> 12) & 0x0F];
            line_buf[pos++] = hex_chars[(half_word >> 8)  & 0x0F];
            line_buf[pos++] = hex_chars[(half_word >> 4)  & 0x0F];
            line_buf[pos++] = hex_chars[half_word & 0x0F];
            line_buf[pos++] = ' ';
        }
        return pos;
    }

    for (size_t i = 0; i < bytes_read; i+=4) {
        const uint32_t word = load_word(&buffer[i], bytes_read - i);

        line_buf[pos++] = hex_chars[(word >> 28) & 0x0F];
        line_buf[pos++] = hex_chars[(word >> 24) & 0x0F];
        line_buf[pos++] = hex_chars[(word >> 20) & 0x0F];
        line_buf[pos++] = hex_chars[(word >> 16) & 0x0F];
        line_buf[pos++] = hex_chars[(word >> 12) & 0x0F];
        line_buf[pos++] = hex_chars[(word >> 8)  & 0x0F];
        line_buf[pos++] = hex_chars[(word >> 4)  & 0x0F];
        line_buf[pos++] = hex_chars[word & 0x0F];
        line_buf[pos++] = ' ';
    }
    return pos;
}

static size_t write_oct_dump(char *line_buf, const uint8_t *buffer, const size_t bytes_read) {
    size_t pos = 0;

    if (output == O_BYTE) {
        for (size_t i = 0; i < bytes_read; i++) {
            line_buf[pos++] = oct_chars[(buffer[i] >> 6) & 0x07];
            line_buf[pos++] = oct_chars[(buffer[i] >> 3) & 0x07];
            line_buf[pos++] = oct_chars[buffer[i] & 0x07];

            line_buf[pos++] = ' ';
        }
        return pos;
    }

    if (output == O_HALF_WORD) {
        for (size_t i = 0; i < bytes_read; i+=2) {
            const uint16_t half_word = load_half_word(&buffer[i], bytes_read - i);

            line_buf[pos++] = oct_chars[(half_word >> 15) & 0x07];
            line_buf[pos++] = oct_chars[(half_word >> 12) & 0x07];
            line_buf[pos++] = oct_chars[(half_word >> 9)  & 0x07];
            line_buf[pos++] = oct_chars[(half_word >> 6)  & 0x07];
            line_buf[pos++] = oct_chars[(half_word >> 3)  & 0x07];
            line_buf[pos++] = oct_chars[half_word & 0x07];

            line_buf[pos++] = ' ';
        }
        return pos;
    }

    for (size_t i = 0; i < bytes_read; i+=4) {
        const uint32_t word = load_word(&buffer[i], bytes_read - i);

        line_buf[pos++] = oct_chars[(word >> 30) & 0x07];
        line_buf[pos++] = oct_chars[(word >> 27) & 0x07];
        line_buf[pos++] = oct_chars[(word >> 24) & 0x07];
        line_buf[pos++] = oct_chars[(word >> 21) & 0x07];
        line_buf[pos++] = oct_chars[(word >> 18) & 0x07];
        line_buf[pos++] = oct_chars[(word >> 15) & 0x07];
        line_buf[pos++] = oct_chars[(word >> 12) & 0x07];
        line_buf[pos++] = oct_chars[(word >> 9)  & 0x07];
        line_buf[pos++] = oct_chars[(word >> 6)  & 0x07];
        line_buf[pos++] = oct_chars[(word >> 3)  & 0x07];
        line_buf[pos++] = oct_chars[word & 0x07];

        line_buf[pos++] = ' ';
    }
    return pos;
}

static size_t write_signed_dump(char *line_buf, const uint8_t *buffer, const size_t bytes_read) {
    size_t pos = 0;

    if (output == O_BYTE) {
        for (size_t i = 0; i < bytes_read; i++) {
            pos += snprintf(&line_buf[pos], MAX_LINE_BUF_LEN - pos, "%4d ", (int8_t)buffer[i]);
        }
        return pos;
    }

    if (output == O_HALF_WORD) {
        for (size_t i = 0; i < bytes_read; i+=2) {
            const uint16_t half_word = load_half_word(&buffer[i], bytes_read - i);
            pos += snprintf(&line_buf[pos], MAX_LINE_BUF_LEN - pos, "%6d ", (int16_t)half_word);
        }
        return pos;
    }

    for (size_t i = 0; i < bytes_read; i+=4) {
        const uint32_t word = load_word(&buffer[i], bytes_read - i);
        pos += snprintf(&line_buf[pos], MAX_LINE_BUF_LEN - pos, "%11d ", (int32_t)word);
    }
    return pos;
}

static size_t write_unsigned_dump(char *line_buf, const uint8_t *buffer, const size_t bytes_read)
{
    size_t pos = 0;

    if (output == O_BYTE) {
        for (size_t i = 0; i < bytes_read; i++) {
            pos += snprintf(&line_buf[pos], MAX_LINE_BUF_LEN - pos, "%3u ", buffer[i]);
        }
        return pos;
    }

    if (output == O_HALF_WORD) {
        for (size_t i = 0; i < bytes_read; i+=2) {
            const uint16_t half_word = load_half_word(&buffer[i], bytes_read - i);
            pos += snprintf(&line_buf[pos], MAX_LINE_BUF_LEN - pos, "%5u ", half_word);
        }
        return pos;
    }

    for (size_t i = 0; i < bytes_read; i+=4) {
        const uint32_t word = load_word(&buffer[i], bytes_read - i);
        pos += snprintf(&line_buf[pos], MAX_LINE_BUF_LEN - pos, "%10u ", word);
    }
    return pos;
}

static void calculate_gap_and_padding(size_t *gap, size_t *pad_chars)
{
    switch (output) {
    case O_BYTE: {
        switch (format) {
        case F_HEX: *pad_chars = 3; break;
        case F_SIGNED: *pad_chars = 5; break;
        default: *pad_chars = 4; break;
        }
        break;
    }
    case O_HALF_WORD: {
        *gap = *gap / 2;
        switch (format) {
        case F_HEX:
            *pad_chars = 5;
            break;
        case F_UNSIGNED:
            *pad_chars = 6;
            break;
        default:
            *pad_chars = 7;
            break;
        }
        break;
    }
    case O_WORD: {
        *gap = *gap / 4;
        switch (format) {
        case F_HEX:
            *pad_chars = 9;
            break;
        case F_UNSIGNED:
            *pad_chars = 11;
            break;
        default:
            *pad_chars = 12;
            break;
            }
        }
    }
}

static void print_elide_line(const uint32_t n_lines)
{
    /* We need the length of msg to calculate padding,
     * so format the message into a temporary buffer. */
    char msg[128];
    int msg_len = snprintf(msg, sizeof(msg), "   *** %u line%s of zero-bytes elided ***",
        n_lines, n_lines == 1 ? "" : "s");

    /* Print the left well (12 spaces). */
    for (int i = 0; i < WELL_WIDTH; i++) {
        printf(" ");
    }

    /* Print the elision message. */
    printf("%s", msg);

    /* Calculate and print the remaining gap to the next border. */
    if (msg_len < bin_width) {
        int gap = bin_width - msg_len;
        for (int i = 0; i < gap; i++) {
            printf(" ");
        }
    }

    /* Pad the ASCII section. */
    for (int i = 0; i < line_width + 2; i++) {
        printf(" ");
    }

    /* Print the final newline. */
    printf("\n");
}

/* Write the binary dump section of output. */
static void write_binary_dump(const uint8_t *buffer, const size_t bytes_read)
{
    char line_buf[MAX_LINE_BUF_LEN];
    size_t pos = 0;

    switch (format) {
    case F_HEX:
        pos = write_hex_dump(line_buf, buffer, bytes_read);
        break;
    case F_OCT:
        pos = write_oct_dump(line_buf, buffer, bytes_read);
        break;
    case F_UNSIGNED:
        pos = write_unsigned_dump(line_buf, buffer, bytes_read);
        break;
    case F_SIGNED:
        pos = write_signed_dump(line_buf, buffer, bytes_read);
        break;
    default:
        break;
    }

    /* Handle the padding for partial lines. */
    if (bytes_read < line_width) {
        size_t gap = line_width - bytes_read;
        size_t pad_chars = 0;

        calculate_gap_and_padding(&gap, &pad_chars);

        memset(&line_buf[pos], ' ', gap * pad_chars);
        pos += gap * pad_chars;
    }

    fwrite(line_buf, 1, pos, stdout);
    printf(" ");
}

static void write_ascii(const uint8_t *buffer, const size_t bytes_read)
{
    if (!ascii) {
        printf("\n");
        return;
    }

    for (size_t i = 0; i < bytes_read; i++) {
        if (buffer[i] >= 0x20 && buffer[i] < 0x7F) {
            putchar(buffer[i]);
        } else {
            printf("%lc", MID_DOT);
        }
    }

    /* Handle the padding gap. */
    if (bytes_read < line_width) {
        const size_t gap = line_width - bytes_read;
        for (size_t i = 0; i < gap; i++) {
            putchar(' ');
        }
    }
    printf("\n");
}

/* Write the output. */
static void write_output(const uint8_t *buffer, const uint64_t offset, const size_t bytes_read)
{
    if (write_well(offset, bytes_read)) {
        /* Write the vertical bars for the last line. */
        for (int i = 0; i < bin_width; i++) {
            printf(" ");
        }

        for (size_t i = 0; i < line_width; i++) {
            printf(" ");
        }
        printf("  ");
        return;
    }
    write_binary_dump(buffer, bytes_read);

    write_ascii(buffer, bytes_read);
}

static int64_t validate_numeric_arg(const char* arg, const int32_t max_val, const char* flag)
{
    /* 'Special value' 0 for base is interpreted as decimal,
     * or hex/oct if 0x or 0 prefix is present. */
    char* p;
    errno = 0;
    const long int value = strtol(arg, &p, 0);

    if (errno == ERANGE) {
        if (value == LONG_MAX) {
            fprintf(stderr, "Error: Overflow occurred.\n");
            exit(EXIT_FAILURE);
        }
        if (value == LONG_MIN) {
            fprintf(stderr, "Error: Underflow occurred.\n");
            exit(EXIT_FAILURE);
        }
    }

    if (p == arg) {
        fprintf(stderr, "Error: No digits were found.\n");
        exit(EXIT_FAILURE);
    }

    if (*p != '\0') {
        fprintf(stderr, "Partial conversion: Number is %ld, but trailing junk found: '%s'\n", value, p);
        exit(EXIT_FAILURE);
    }

    if (value < 0 ) {
        fprintf(stderr, "Negative values not valid for %s\n", flag);
        exit(EXIT_FAILURE);
    }

    if (max_val != 0 && value > max_val) {
        fprintf(stderr, "Argument too large for %s\n", flag);
        exit(EXIT_FAILURE);
    }
    return value;
}

int main(const int argc, char *argv[])
{
    setlocale(LC_ALL, "");
    int opt;
    uint64_t offset = 0;

    /* Zeroed-out memory to compare for lines of just NUL bytes. */
    static constexpr uint8_t zero_block[256] = {0};
    /* Counter of elided lines. */
    uint32_t n_elided = 0;
    /* Flag for whether to elide or not. */
    bool elide = true;

    const struct option longopts[] = {
        { .name = "hex",           .has_arg = no_argument,       .flag = nullptr, .val = 'x' },
        { .name = "oct",           .has_arg = no_argument,       .flag = nullptr, .val = 'o' },
        { .name = "unsigned",      .has_arg = no_argument,       .flag = nullptr, .val = 'd' },
        { .name = "signed",        .has_arg = no_argument,       .flag = nullptr, .val = 'S' },
        { .name = "byte",          .has_arg = no_argument,       .flag = nullptr, .val = 'b' },
        { .name = "half-word",     .has_arg = no_argument,       .flag = nullptr, .val = 'H' },
        { .name = "word",          .has_arg = no_argument,       .flag = nullptr, .val = 'W' },
        { .name = "big-endian",    .has_arg = no_argument,       .flag = nullptr, .val = 'B' },
        { .name = "little-endian", .has_arg = no_argument,       .flag = nullptr, .val = 'L' },
        { .name = "no-elide",      .has_arg = no_argument,       .flag = nullptr, .val = 'n' },
        { .name = "skip-bytes",    .has_arg = required_argument, .flag = nullptr, .val = 's' },
        { .name = "read-bytes",    .has_arg = required_argument, .flag = nullptr, .val = 'r' },
        { .name = "line-width",    .has_arg = required_argument, .flag = nullptr, .val = 'l' },
        { .name = "help",          .has_arg = no_argument,       .flag = nullptr, .val = 'h' },
        { .name = "version",       .has_arg = no_argument,       .flag = nullptr, .val = 'V' },
        { .name = "ascii",         .has_arg = no_argument,       .flag = nullptr, .val = 'a' },
        { .name = nullptr,         .has_arg = no_argument,       .flag = nullptr, .val = 0 }
    };

    while ((opt = getopt_long(argc, argv, "xodSbHWBLbns:r:l:hVa", longopts, nullptr)) != -1) {
        switch(opt) {
        case 'x':
            format = F_HEX;
            break;
        case 'd':
            format = F_UNSIGNED;
            break;
        case 'S':
            format = F_SIGNED;
            break;
        case 'o':
            format = F_OCT;
            break;
        case 'b':
            output = O_BYTE;
            break;
        case 'H':
            output = O_HALF_WORD;
            break;
        case 'W':
            output = O_WORD;
            break;
        case 'L':
            little_endian = true;
            break;
        case 'B':
            little_endian = false;
            break;
        case 'n':
            elide = false;
            break;
        case 'a':
            ascii = true;
            break;
        case 's':
            offset = (int32_t)validate_numeric_arg(optarg, 0, "--skip-bytes");
            break;
        case 'r':
            read_size = (size_t)validate_numeric_arg(optarg, 0, "--read-bytes");
            break;
        case 'l':
            line_width = (uint8_t)validate_numeric_arg(optarg, 255, "--line-width");
            break;
            case 'V':
                printf("%s (%s) version %s\n", APP_NAME, APP_SUITE, APP_VERSION);
                printf("%s compiled on %s at %s\n",
                       strrchr(__FILE__, '/') ? strrchr(__FILE__, '/') + 1 : __FILE__,
                       __DATE__, __TIME__);
                return EXIT_SUCCESS;
            case 'h':
                show_help();
                return EXIT_SUCCESS;
            default:
                show_help();
                return EXIT_FAILURE;
        }
    }

    /* Normalize options. */

    /* Half-word output requires line_width be divisible by 2. */
    if (output == O_HALF_WORD) {
        line_width = line_width & ~1;
    }
    /* Full-word output requires line_width be divisible by 4. */
    if (output == O_WORD) {
        line_width = (line_width + 2) & ~3;
    }

    /* Calculate and cache bin width. */
    bin_width = get_bin_width();

    /* Open arg/stdin for reading. */
    FILE* input;

    /* Ensure we only call fopen() on passed args,
     * and not on shell I/O redirects. */
    if (optind < argc) {
        input = fopen(argv[optind], "rb");
        if (!input) {
            fprintf(stderr, "failed to open %s: %s\n",
                argv[optind], strerror(errno));
            exit(EXIT_FAILURE);
        }
    } else {
        input = stdin;
    }

    /* Get file size if read_size not set. */
    if (read_size == 0) {
        read_size = get_file_size(fileno(input));
    }

    /* Call fseek() if --start-offset is used. */
    if (offset != 0) {
        if (fseeko(input, (off_t)offset, SEEK_SET) < 0) {
            /* fseek() fails on pipes. We must manually consume and discard
             * 'offset' bytes to reach the correct starting position in the stream. */
            size_t bytes_to_discard = offset;
            uint8_t discard_buf[CHUNK_SIZE];

            while (bytes_to_discard > 0) {
                const size_t grab = (bytes_to_discard < sizeof(discard_buf)) ? bytes_to_discard : sizeof(discard_buf);
                const size_t read_in = fread(discard_buf, 1, grab, input);

                if (read_in == 0) {
                    break; /* EOF reached before we even hit the offset. */
                }
                bytes_to_discard -= read_in;
            }
        }
    }


    /* This forces printf to buffer CHUNK_SIZE before calling write(). */
    char stdout_buffer[CHUNK_SIZE];
    setvbuf(stdout, stdout_buffer, _IOFBF, sizeof(stdout_buffer));

    uint8_t file_buf[CHUNK_SIZE];

    while (read_size > 0) {
        /* Determine how much to read into the big block. */
        const size_t to_read = (read_size < CHUNK_SIZE) ? read_size : CHUNK_SIZE;
        const size_t bytes_read = fread(file_buf, 1, to_read, input);

        if (bytes_read == 0) break;

        size_t i = 0;
        /* Slice the big block into line_width chunks. */
        while (i < bytes_read) {
            const size_t chunk_len = (bytes_read - i < line_width) ? bytes_read - i : line_width;

            if (elide) {
                const bool is_zero = memcmp(&file_buf[i], zero_block, chunk_len) == 0;
                if (is_zero) {
                    n_elided++;
                    if (n_elided == 1) {
                        /* It's the first row of zeros. Print it normally. */
                        write_output(&file_buf[i], offset, chunk_len);
                    }
                } else {
                    if (n_elided > 1) {
                        /* Already printed the first one, so we actually skipped (n_elided - 1). */
                        print_elide_line(n_elided - 1);
                    }

                    n_elided = 0;

                    /* Print the current non-zero row. */
                    write_output(&file_buf[i], offset, chunk_len);
                }
            } else {
                write_output(&file_buf[i], offset, chunk_len);
            }

            offset += chunk_len;
            i += chunk_len;
        }
        read_size -= bytes_read;
    }

    if (n_elided > 1) {
        print_elide_line(n_elided - 1);
    }

    fclose(input);

    return EXIT_SUCCESS;
}
