#include "utf8transcoder.h"

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

