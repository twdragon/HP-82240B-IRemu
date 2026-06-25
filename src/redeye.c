/*
 * redeye.c
 * ─────────────
 * HP 82240B RedEye frame encoder and PIO FIFO interface.
 */

#include "redeye.h"

#include "hardware/pio.h"
#include "hardware/gpio.h"

/* =========================================================================
 * W board LED helper functions
 * =========================================================================
 * Control Pico W board LED circuit 
 */

int pico_led_init(void) 
{
#if defined(PICO_DEFAULT_LED_PIN)
    gpio_init(PICO_DEFAULT_LED_PIN);
    gpio_set_dir(PICO_DEFAULT_LED_PIN, GPIO_OUT);
    return PICO_OK;
#elif defined(CYW43_WL_GPIO_LED_PIN)
    return cyw43_arch_init();
#endif
}

void pico_set_led(bool led_on) 
{
#if defined(PICO_DEFAULT_LED_PIN)
    gpio_put(PICO_DEFAULT_LED_PIN, led_on);
#elif defined(CYW43_WL_GPIO_LED_PIN)
    cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, led_on);
#endif
}

const uint8_t redeye_ecc(uint8_t c)
{ // https://stackoverflow.com/a/25808559/9560245
    static const uint8_t nt [16] =
    {
        0, 1, 1, 2, 1, 2, 2, 3, 
        1, 2, 2, 3, 2, 3, 3, 4
    };
    uint8_t ecc = 0x00;
    uint8_t bs = 0x00;
    bs = c & 0b10001011; // Parity 0
    ecc = (nt[bs & 0x0F] + nt[bs >> 4]) & 0x01;
    bs = c & 0b11010101; // Parity 1
    ecc |= ((nt[bs & 0x0F] + nt[bs >> 4]) & 0x01) << 1;
    bs = c & 0b11100110; // Parity 2
    ecc |= ((nt[bs & 0x0F] + nt[bs >> 4]) & 0x01) << 2;
    bs = c & 0b01111000; // Parity 3
    ecc |= ((nt[bs & 0x0F] + nt[bs >> 4]) & 0x01) << 3;
    return ecc;
}

const uint16_t redeye_frame(uint8_t c)
{
    return (0x0000 | (redeye_ecc(c) << 8) | c) << 4;
}

uint8_t redeye_transcode_latin1(const uint32_t uc)
{
    uint8_t c = REDEYE_FILLER;
    uint8_t* codes = (uint8_t*)&uc;
    if((uc >= 0x20) && (uc <= 0x7E)) // ASCII part
        c = codes[0];
    else if((codes[1] == 0xC2) && 
            ((codes[0] >= 0xA0) && (codes[0] <= 0xFF))) // ISO8859-1 part
        c = codes[0];
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
    else if((codes[1] == 0x04) && (codes[0] == 0x51)) // Cyrillic Ё
        c = 203;
    else if((codes[1] == 0x04) && (codes[0] == 0x01)) // Cyrillic ё
        c = 235;
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
        }
    }
    return c;
}

void redeye_set_word_wrap(void)
{
    redeye_linewrap = true;
}

void redeye_stop_word_wrap(void)
{
    redeye_linewrap = false;
}

