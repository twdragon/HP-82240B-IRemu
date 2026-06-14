#ifndef UTF8TRANSCODER_H
#define UTF8TRANSCODER_H

#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <stddef.h>

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

void utf8_unpack_tables(void);
int utf8_valid(const uint8_t *m, size_t len);
size_t utf8_to_utf32(const uint8_t *m,
                     size_t mlen, 
                     uint32_t *out);

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

#endif /* UTF8TRANSCODER_H */
