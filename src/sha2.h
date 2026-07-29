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


uint32_t *words;
uint64_t *l_words;
uint8_t  *in_buf;

/* The eight 32-bit unsigned registers used by sha224/256. */
uint32_t reg_h0;
uint32_t reg_h1;
uint32_t reg_h2;
uint32_t reg_h3;
uint32_t reg_h4;
uint32_t reg_h5;
uint32_t reg_h6;
uint32_t reg_h7;

/* The eight 64-bit unsigned registers used by sha384/512. */
uint64_t reg64_h0;
uint64_t reg64_h1;
uint64_t reg64_h2;
uint64_t reg64_h3;
uint64_t reg64_h4;
uint64_t reg64_h5;
uint64_t reg64_h6;
uint64_t reg64_h7;

/* A 64-element array k32[1 ... 64] constructed from the
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

/* An 80-element array k64[1 ... 80] constructed from the
 * first 64 bits of the fractional parts of the cube
 * roots of the first 80 primes 2..409. */
constexpr uint64_t k64[] = { 0x428a2f98d728ae22, 0x7137449123ef65cd, 0xb5c0fbcfec4d3b2f, 0xe9b5dba58189dbbc,
                             0x3956c25bf348b538, 0x59f111f1b605d019, 0x923f82a4af194f9b, 0xab1c5ed5da6d8118,
                             0xd807aa98a3030242, 0x12835b0145706fbe, 0x243185be4ee4b28c, 0x550c7dc3d5ffb4e2,
                             0x72be5d74f27b896f, 0x80deb1fe3b1696b1, 0x9bdc06a725c71235, 0xc19bf174cf692694,
                             0xe49b69c19ef14ad2, 0xefbe4786384f25e3, 0x0fc19dc68b8cd5b5, 0x240ca1cc77ac9c65,
                             0x2de92c6f592b0275, 0x4a7484aa6ea6e483, 0x5cb0a9dcbd41fbd4, 0x76f988da831153b5,
                             0x983e5152ee66dfab, 0xa831c66d2db43210, 0xb00327c898fb213f, 0xbf597fc7beef0ee4,
                             0xc6e00bf33da88fc2, 0xd5a79147930aa725, 0x06ca6351e003826f, 0x142929670a0e6e70,
                             0x27b70a8546d22ffc, 0x2e1b21385c26c926, 0x4d2c6dfc5ac42aed, 0x53380d139d95b3df,
                             0x650a73548baf63de, 0x766a0abb3c77b2a8, 0x81c2c92e47edaee6, 0x92722c851482353b,
                             0xa2bfe8a14cf10364, 0xa81a664bbc423001, 0xc24b8b70d0f89791, 0xc76c51a30654be30,
                             0xd192e819d6ef5218, 0xd69906245565a910, 0xf40e35855771202a, 0x106aa07032bbd1b8,
                             0x19a4c116b8d2d0c8, 0x1e376c085141ab53, 0x2748774cdf8eeb99, 0x34b0bcb5e19b48a8,
                             0x391c0cb3c5c95a63, 0x4ed8aa4ae3418acb, 0x5b9cca4f7763e373, 0x682e6ff3d6b2b8a3,
                             0x748f82ee5defb2fc, 0x78a5636f43172f60, 0x84c87814a1f0ab72, 0x8cc702081a6439ec,
                             0x90befffa23631e28, 0xa4506cebde82bde9, 0xbef9a3f7b2c67915, 0xc67178f2e372532b,
                             0xca273eceea26619c, 0xd186b8c721c0c207, 0xeada7dd6cde0eb1e, 0xf57d4f7fee6ed178,
                             0x06f067aa72176fba, 0x0a637dc5a2c898a6, 0x113f9804bef90dae, 0x1b710b35131c471b,
                             0x28db77f523047d84, 0x32caab7b40c72493, 0x3c9ebe0a15c9bebc, 0x431d67c49c100d4c,
                             0x4cc5d4becb3e42b6, 0x597f299cfc657e2a, 0x5fcb6fab3ad6faec, 0x6c44198c4a475817};

/* Right-rotate n by d bits. */
inline extern uint32_t right_rotate_32(const uint32_t n, const uint8_t d)
{
    return (n >> d) | (n << (INT_32_BITS - d));
}

inline extern uint64_t right_rotate_64(const uint64_t n, const uint8_t d)
{
    return (n >> d) | (n << (INT_64_BITS - d));
}

inline extern void encode_words_32(const uint8_t *chunk)
{
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
                            (words[i - 15] >> 3);
        const uint32_t s1 = right_rotate_32(words[i - 2], 17)  ^
                            right_rotate_32(words[i - 2], 19)  ^
                            (words[i - 2] >> 10);
        words[i] = words[i - 16] + s0 + words[i - 7] + s1;
    }
}

inline extern void encode_words_64(const uint8_t *chunk) {
    /* Combine bytes into 64-bit big-endian words. */
    for (int j = 0; j < 16; j++) {
        l_words[j] = ((uint64_t)chunk[8 * j]     << 56) |
                     ((uint64_t)chunk[8 * j + 1] << 48) |
                     ((uint64_t)chunk[8 * j + 2] << 40) |
                     ((uint64_t)chunk[8 * j + 3] << 32) |
                     ((uint64_t)chunk[8 * j + 4] << 24) |
                     ((uint64_t)chunk[8 * j + 5] << 16) |
                     ((uint64_t)chunk[8 * j + 6] <<  8) |
                      (uint64_t)chunk[8 * j + 7];
    }

    /* Extend first 16 words into the remaining 64
     * words w[16..79] of the message schedule array. */
    for (int i = 16; i < 80; i++) {
        const uint64_t s0 = right_rotate_64(l_words[i - 15], 1) ^
                            right_rotate_64(l_words[i - 15], 8) ^
                            (l_words[i - 15] >> 7);
        const uint64_t s1 = right_rotate_64(l_words[i - 2], 19) ^
                            right_rotate_64(l_words[i - 2], 61) ^
                            (l_words[i - 2] >> 6);
        l_words[i] = l_words[i - 16] + s0 + l_words[i - 7] + s1;
    }
}

inline extern void process_chunk_32(const uint8_t *chunk)
{
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
        const uint32_t ch = g ^ (e & (f ^ g));
        const uint32_t temp1 = h + S1 + ch + k32[i] + words[i];
        const uint32_t S0 = right_rotate_32(a, 2) ^ right_rotate_32(a, 13) ^ right_rotate_32(a, 22);
        const uint32_t maj = (a & b) ^ (c & (a ^ b));
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

inline extern void process_chunk_64(const uint8_t *chunk)
{
    encode_words_64(chunk);

    uint64_t a = reg64_h0;
    uint64_t b = reg64_h1;
    uint64_t c = reg64_h2;
    uint64_t d = reg64_h3;
    uint64_t e = reg64_h4;
    uint64_t f = reg64_h5;
    uint64_t g = reg64_h6;
    uint64_t h = reg64_h7;

    for (int i = 0; i < 80; i++) {
        const uint64_t S1 = right_rotate_64(e, 14) ^ right_rotate_64(e, 18) ^ right_rotate_64(e, 41);
        const uint64_t ch = g ^ (e & (f ^ g));
        const uint64_t temp1 = h + S1 + ch + k64[i] + l_words[i];
        const uint64_t S0 = right_rotate_64(a, 28) ^ right_rotate_64(a, 34) ^ right_rotate_64(a, 39);
        const uint64_t maj = (a & b) ^ (c & (a ^ b));
        const uint64_t temp2 = S0 + maj;

        h = g;
        g = f;
        f = e;
        e = d + temp1;
        d = c;
        c = b;
        b = a;
        a = temp1 + temp2;
    }

    reg64_h0 += a;
    reg64_h1 += b;
    reg64_h2 += c;
    reg64_h3 += d;
    reg64_h4 += e;
    reg64_h5 += f;
    reg64_h6 += g;
    reg64_h7 += h;
}

#endif
