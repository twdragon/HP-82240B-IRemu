#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* =========================================================================
 * ECC EVEN PARITY CODE LOOKUP TABLE
 * -----------------------------------------------------
 * 11  10  9   8   |   7   6   5   4   3   2   1   0
 *  +              |       +   +   +   +               3
 *      +          |   +   +   +           +   +       2
 *         +       |   +   +       +       +       +   1
 *             +   |   +               +       +   +   0
 * -----------------------------------------------------
 * https://stackoverflow.com/a/25808559/9560245
*/

static inline const uint8_t redeye_ecc(uint8_t c)
{
    static const uint8_t nt [16] =
    {
        0, 1, 1, 2, 1, 2, 2, 3, 
        1, 2, 2, 3, 2, 3, 3, 4
    };
    uint8_t ecc = 0x00;
    uint8_t bs = 0x00;
    bs = c & 0b10001011; // Parity 0
    ecc = ((nt[bs & 0x0F] + nt[bs >> 4]) & 0x01);
    bs = c & 0b11010101; // Parity 1
    ecc |= (((nt[bs & 0x0F] + nt[bs >> 4]) & 0x01)) << 1;
    bs = c & 0b11100110; // Parity 2
    ecc |= (((nt[bs & 0x0F] + nt[bs >> 4]) & 0x01)) << 2;
    bs = c & 0b01111000; // Parity 3
    ecc |= (((nt[bs & 0x0F] + nt[bs >> 4]) & 0x01)) << 3;
    return ecc;
}

const uint16_t redeye_frame(uint8_t c)
{
    return (0x0000 | (redeye_ecc(c) << 8) | c) << 4;
}

int main(int argc, char* argv[]) 
{
    const uint8_t char_A = 0b01000001; // DEC 65
    return redeye_frame(char_A) == 0b1101010000010000 ? 0 : 1;
}
