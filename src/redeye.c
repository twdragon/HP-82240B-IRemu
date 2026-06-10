/*
 * redeye.c
 * ─────────────
 * HP 82240B RedEye frame encoder and PIO FIFO interface.
 *
 * This module does NOT initialise hardware.  PIO setup lives in main().
 * Call hp_proto_set_pio() once after the SM is running.
 */

#include "redeye.h"

#include "hardware/pio.h"
#include "hardware/gpio.h"

/* =========================================================================
 * W board LED helper functions
 * =========================================================================
 * Control Pico W board LED circuit */

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

void redeye_putc(const uint8_t c)
{
    pico_set_led(true);
    pio_sm_put_blocking(pio, sm, (redeye_frame(c) << 16) );
    pico_set_led(false);
}

void redeye_putesc(const uint8_t c)
{
    pico_set_led(true);
    pio_sm_put_blocking(pio, sm, (redeye_frame(REDEYE_ESCAPE) << 16) );
    pico_set_led(false);
    redeye_putc(c);
}
