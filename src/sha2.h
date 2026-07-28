/***************************************************************************
 *   sha2.h - functions shared by the shaNsum programs                     *
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

#ifndef SHA2_H
#define SHA2_H

#include <stdint.h>

#define INT_32_BITS 32
#define INT_64_BITS 64


extern uint32_t *words;
extern uint8_t  *in_buf;

/* The eight 32-bit unsigned registers used by sha224/256. */
extern uint32_t reg_h0;
extern uint32_t reg_h1;
extern uint32_t reg_h2;
extern uint32_t reg_h3;
extern uint32_t reg_h4;
extern uint32_t reg_h5;
extern uint32_t reg_h6;
extern uint32_t reg_h7;

/* A 64-element array k[1 ... 64] constructed from the
 * first 32 bits of the fractional parts of the cube
 * roots of the first 64 primes 2..311. */
constexpr uint32_t k32[] = { 0x428a2f98, 0x71374491, 0xb5c0fbcf, 0xe9b5dba5,
                             0x3956c25b, 0x59f111f1, 0x923f82a4, 0xab1c5ed5,
                             0xd807aa98, 0x12835b01, 0x243185be, 0x550c7dc3,
                             0x72be5d74, 0x80deb1fe, 0x9bdc06a7, 0xc19bf174,
                             0xe49b69c1, 0xefbe4786, 0x0fc19dc6, 0x240ca1cc,
                             0x2de92c6f, 0x4a7484aa, 0x5cb0a9dc, 0x76f988da,
                             0x983e5152, 0xa831c66d, 0xb00327c8, 0xbf597fc7,
                             0xc6e00bf3, 0xd5a79147, 0x06ca6351, 0x14292967,
                             0x27b70a85, 0x2e1b2138, 0x4d2c6dfc, 0x53380d13,
                             0x650a7354, 0x766a0abb, 0x81c2c92e, 0x92722c85,
                             0xa2bfe8a1, 0xa81a664b, 0xc24b8b70, 0xc76c51a3,
                             0xd192e819, 0xd6990624, 0xf40e3585, 0x106aa070,
                             0x19a4c116, 0x1e376c08, 0x2748774c, 0x34b0bcb5,
                             0x391c0cb3, 0x4ed8aa4a, 0x5b9cca4f, 0x682e6ff3,
                             0x748f82ee, 0x78a5636f, 0x84c87814, 0x8cc70208,
                             0x90befffa, 0xa4506ceb, 0xbef9a3f7, 0xc67178f2 };

/* Right-rotate n by d bits. */
inline extern uint32_t right_rotate_32(const uint32_t n, const uint8_t d)
{
    return (n >> d) | (n << (INT_32_BITS - d));
}

inline extern void encode_words_32(const uint8_t *chunk) {
    /* Combine bytes into 32-bit big-endian words. */
    for (int j = 0; j < 16; j++) {
        words[j] = ((uint32_t)chunk[4 * j]     << 24) |
                   ((uint32_t)chunk[4 * j + 1] << 16) |
                   ((uint32_t)chunk[4 * j + 2] <<  8) |
                    (uint32_t)chunk[4 * j + 3];
    }

    /* Extend the first 16 words into the remaining 48
     * words w[16..63] of the message schedule array. */
    for (int i = 16; i < 64; i++) {
        const uint32_t s0 = right_rotate_32(words[i - 15], 7)  ^
                            right_rotate_32(words[i - 15], 18) ^
                            (words[i-15] >> 3);
        const uint32_t s1 = right_rotate_32(words[i - 2], 17)  ^
                            right_rotate_32(words[i - 2], 19)  ^
                            (words[i - 2] >> 10);
        words[i] = words[i - 16] + s0 + words[i - 7] + s1;
    }
}

inline extern void process_chunk_32(const uint8_t *chunk) {
    encode_words_32(chunk);

    uint32_t a = reg_h0;
    uint32_t b = reg_h1;
    uint32_t c = reg_h2;
    uint32_t d = reg_h3;
    uint32_t e = reg_h4;
    uint32_t f = reg_h5;
    uint32_t g = reg_h6;
    uint32_t h = reg_h7;

    for (int i = 0; i < 64; i++) {
        const uint32_t S1 = right_rotate_32(e, 6) ^ right_rotate_32(e, 11) ^ right_rotate_32(e, 25);
        const uint32_t ch = (e & f) ^ ((~e) & g);
        const uint32_t temp1 = h + S1 + ch + k32[i] + words[i];
        const uint32_t S0 = right_rotate_32(a, 2) ^ right_rotate_32(a, 13) ^ right_rotate_32(a, 22);
        const uint32_t maj = (a & b) ^ (a & c) ^ (b & c);
        const uint32_t temp2 = S0 + maj;

        h = g;
        g = f;
        f = e;
        e = d + temp1;
        d = c;
        c = b;
        b = a;
        a = temp1 + temp2;
    }

    reg_h0 += a;
    reg_h1 += b;
    reg_h2 += c;
    reg_h3 += d;
    reg_h4 += e;
    reg_h5 += f;
    reg_h6 += g;
    reg_h7 += h;
}


#endif
