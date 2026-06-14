#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <stddef.h>

/*
 * USES PARTIALLY THE DECODER ROUTINE BY ANDREW MOON
 * https://github.com/floodyberry/utf8dfadecoder

MIT License

Copyright (c) 2025 Andrew Moon

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/ 

#define utf_replacement 0xfffd

typedef struct utf8_decode_state_t 
{
    uint32_t state, c;
} utf8_decode_state;

#define UTF8_TABLE_MULTIPLIER 256

static uint8_t utf8_leading_byte_value[256];
static uint8_t utf8_state_table[256 * 21];

static const uint16_t utf8_state_table_packed[49] = {
	0x8780,0xc45d,0x2aa8,0xc62c,0x0b42,0xcc2e,0x0720,0x670d,
	0x9220,0x1044,0x83c2,0xfa2d,0xa2d8,0x2d87,0xd97a,0x9ba2,
	0xfa2d,0xa2d9,0x2da3,0xb17a,0x8ac3,0xb63e,0xc3e8,0x2b16,
	0xd8fa,0x0fa2,0xc2fc,0x3038,0x0e88,0xb060,0x8a00,0x720e,
	0xa210,0x0d0f,0xe883,0xc5e4,0x5e40,0xe40c,0x40c5,0x0c5e,
	0x1ba2,0x5e48,0xe40c,0x40c5,0x0c5e,0x93a2,0x8317,0x62b9,
	0x0003
};

static const uint8_t utf8_state_table_lengths[9] = {
	0x1e,0x20,0x30,0x37,0x3e,0x3f,0x40,0x80,0xc0
};

#define utf8_accept 0
#define utf8_reject 1
#define utf8_fail   2

void utf8_unpack_tables(void) 
{
	uint32_t i, j, c, count, val, p;
	size_t bitsleft;

for (c = 0, i = 128; i != 0; i >>= 1)
    for (j = 0; j < i; j++)
        utf8_leading_byte_value[c++] = (i != 64) ? (uint8_t)j : 0;
    utf8_leading_byte_value[c] = 0;

    for (c = 0, i = 0, bitsleft = 0, p = 0; c != 256 * 21;) 
    {
        if (bitsleft < 10)
        {
            p |= ((uint32_t)utf8_state_table_packed[i++] << bitsleft);
            bitsleft += 16;
        }
        bitsleft -= 10;
        val   = (p & 0x1f) ; p >>= 5 ;
        count = (p & 0x1f) ; p >>= 5 ;
        count = (count < 21) ? count : (count < 30) ? utf8_state_table_lengths[count-21] : 640;
        while (count--)
            utf8_state_table[c++] = val;
	}
}

int utf8_valid(const uint8_t *m, size_t len) 
{
    uint8_t state;
    size_t i;
    for (i = 0, state = 0; i < len; i++)
        state = utf8_state_table[m[i] + (state * UTF8_TABLE_MULTIPLIER)];
    return state == utf8_accept;
}

static inline const uint8_t * utf8_decode_unsafe(const uint8_t *m,
                                                 uint32_t *c)
{
    uint8_t state = utf8_state_table[*m];
    *c = utf8_leading_byte_value[*m++];
    while (state > utf8_fail) 
    {
        state = utf8_state_table[*m + (state * UTF8_TABLE_MULTIPLIER)];
        *c = (*c << 6) | (*m++ & 0x3f);
    }
    if (state != utf8_accept) 
    {
        *c = utf_replacement;
        m -= (state == utf8_fail);
    }
    return m;
}

static inline const uint8_t * utf8_decode(const uint8_t *m,
                                          const uint8_t *end,
                                          uint32_t *c)
{
    uint8_t state = utf8_state_table[*m];
    *c = utf8_leading_byte_value[*m++];
    while ((state > utf8_fail) && (m < end)) 
    {
        state = utf8_state_table[*m + (state * UTF8_TABLE_MULTIPLIER)];
        *c = (*c << 6) | (*m++ & 0x3f);
    }
    if (state != utf8_accept)
    {
        *c = utf_replacement;
        m -= (state == utf8_fail);
    }
    return m;
}

size_t utf8_to_utf32(const uint8_t *m,
                     size_t mlen, 
                     uint32_t *out)
{
    const uint8_t *end = m + mlen, *end6 = (mlen >= 6) ? (end - 6) : m;
    uint32_t *start = out, c;

    while (m < end6)
    {
        while ((m < end6) && (*m < 0x80))
        *out++ = (uint32_t)*m++;

        while ((m < end6) && (*m >= 0x80)) 
        {
            m = utf8_decode_unsafe(m, &c);
            *out++ = c;
        }
    }

    while (m < end)
    {
        m = utf8_decode(m, end, &c);
        *out++ = c;
    }

    return out - start;
}

/* END OF DECODER ROUTINE */

int main(int argc, char* argv[])
{
    const char test_cyrillic[] = "∫Я строка←ÿ▶";
    utf8_unpack_tables();
    if(!utf8_valid(test_cyrillic, strlen(test_cyrillic)))
        return 1;
    uint32_t utf8_decoded[12];
    uint32_t utf8_decode_test[12] = {0x222B, 0x042F, 0x0020, 0x0441, 0x0442,
                                     0x0440, 0x043E, 0x043A, 0x0430, 0x2190,
                                     0x00FF, 0x25B6};
    utf8_to_utf32(test_cyrillic, strlen(test_cyrillic), utf8_decoded);
    for(uint8_t i = 0; i < 12; i++)
        if(utf8_decoded[i] != utf8_decode_test[i])
            return 1;
    return 0;
}
