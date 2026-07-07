#include "utf8transcoder.h"
#include "redeye_commands.h"

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

uint8_t redeye_transcode_latin1(const uint32_t uc)
{
    uint8_t c = REDEYE_FILLER;
    uint8_t* codes = (uint8_t*)&uc;
    if((uc >= 0x0A) && (uc <= 0x0D)) // Line feeders
        c = REDEYE_LF;
    else if((uc >= 0x20) && (uc <= 0x7E)) // ASCII part
        c = codes[0];
    else if((codes[1] == 0xC2) && 
            ((codes[0] >= 0xA0) && (codes[0] <= 0xFF))) // ISO8859-1 part
        c = codes[0];
    else if((codes[1] == 0x04) && (codes[0] == 0x51)) // Cyrillic Ё
        c = 203;
    else if((codes[1] == 0x04) && (codes[0] == 0x01)) // Cyrillic ё
        c = 235;
    else if( ((codes[1] == 0x04) && 
              ((codes[0] >= 0x10) && (codes[0] <= 0x8F))) ) // Cyrillic part
    {
        static const uint8_t cyrillic_table[64] = { // Decimal codes
            //А    Б    В    Г    Д    Е    Ж    З
             65,  98,  66, 132,  68,  69, 215,  51,
            //И    Й    К    Л    М    Н    О    П
             73, 209,  75, 182,  77,  72,  79, 156,
            //Р    С    Т    У    Ф    Х    Ц    Ч
             80,  67,  84,  89, 163,  88,  85,  52,
            //Ш    Щ    Ъ    Ы    Ь    Э    Ю    Я
            154, 154, 254, 198, 254, 162, 222, 182,
            //а    б    в    г    д    е    ж    з
             97, 146, 223, 132, 113, 101, 215,  51,
            //и    й    к    л    м    н    о    п
            105, 209, 107, 150,  77,  72, 111, 135,
            //р    с    т    у    ф    х    ц    ч
            112,  99, 116, 121, 149, 120, 117,  52,
            //ш    щ    ъ    ы    ь    э    ю    я
            154, 154, 254, 230, 254, 170, 222, 240
        };
        if(codes[1] == 0x04)
            c = cyrillic_table[codes[0] - 0x10];
    }
    else 
    {
        switch(uc) // Special HP substitution table
        {
        case 0xE288A0:  // Measured angle
            c = 0x80;
            break;
        case 0xC397:    // Lined cross / multiplication
            c = 0x81;
            break;
        case 0xE28887:  // Nabla operator
            c = 0x82;
            break;
        case 0xE2889A:  // Radical sign
            c = 0x83;
            break;
        case 0xE288AB:  // Integral sign
            c = 0x84;
            break;
        case 0xE28891:  // Summation sign
            c = 0x85;
            break;
        case 0xE296B6:  // Black Right-pointing Triangle
            c = 0x86;
            break;
        case 0xCF80:    // pi
        case 0xE1B4A8:
            c = 0x87;
            break;
        case 0xE28882:  // Partial Differential
            c = 0x88;
            break;
        case 0xE289A4:  // Less-than Or Equal To
            c = 0x89;
            break;
        case 0xE289A5:  // Greater-than Or Equal To
            c = 0x8A;
            break;
        case 0xE289A0:  // Not Equal To
            c = 0x8B;
            break;
        case 0xC991:    // alpha
            c = 0x8C;
            break;
        case 0xE28692:  // Rightwards Arrow
            c = 0x8D;
            break;
        case 0xE28690:  // Leftwards Arrow
            c = 0x8E;
            break;
        case 0xE28693:  // Downwards Arrow
            c = 0x8F;
            break;
        case 0xE28691:  // Upwards Arrow
            c = 0x90;
            break;
        case 0xC9A3:    // gamma
            c = 0x91;
            break;
        case 0xCEB4:    // delta
            c = 0x92;
            break;
        case 0xCEB5:    // epsilon
            c = 0x93;
            break;
        case 0xCEB7:    // eta
            c = 0x94;
            break;
        case 0xCEB8:    // theta
            c = 0x95;
            break;
        case 0xCEBB:    // lambda
            c = 0x96;
            break;
        case 0xCF81:    // rho
            c = 0x97;
            break;
        case 0xCF83:    // sigma
            c = 0x98;
            break;
        case 0xCF84:    // tau
            c = 0x99;
            break;
        case 0xCF89:    // omega
            c = 0x9A;
            break;
        case 0xE28886:  // Increment Sign
            c = 0x9B;
            break;
        case 0xE2888F:  // N-ary Product Sign
            c = 0x9C;
            break;
        case 0xE284A6:  // Ohm Sign
            c = 0x9D;
            break;
        case 0xE2888E:  // End of Proof
            c = 0x9E;
            break;
        case 0xE2889E:  // Infinity
        case 0xC9B7:    // Closed omega
            c = 0x9F;
            break;
        case 0x09:      // TAB replaced by single space
            c = 0x20;
            break;
        }
    }
    return c;
}
