#ifndef REDEYE_H
#define REDEYE_H

#include "pico/stdlib.h"
#include "pico/bit_ops.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <stddef.h>
#include <assert.h>

#include "hardware/pio.h"
#include "hardware/gpio.h"

#ifdef CYW43_WL_GPIO_LED_PIN 
    #include "pico/cyw43_arch.h"
#endif

#include "redeye.pio.h"

#define IR_TRANSMITTER_PIN 5

#define SIGNAL_PIN_OFFSET 2
#define IR_SIGNAL_PIN (IR_TRANSMITTER_PIN - SIGNAL_PIN_OFFSET)

/* Printer hardware definition constants */
#define CHAR_LINE_WIDTH 24
#define WCHAR_LINE_WIDTH 12
#define DOT_CHAR_WIDTH 6
#define DOT_CHAR_HEIGHT 8
#define DOT_LINE_WIDTH (DOT_CHAR_WIDTH * CHAR_LINE_WIDTH)

/* RedEye protocol command table */
#define REDEYE_ESCAPE 27
#define REDEYE_CR     4
#define REDEYE_LF     10
#define REDEYE_RESET  255
#define REDEYE_TEST   254
#define REDEYE_SET2X  253
#define REDEYE_END2X  252
#define REDEYE_SETUL  251
#define REDEYE_ENDUL  250
#define REDEYE_ECMA94 249 // ISO8859-1 [Latin-1]
#define REDEYE_ROMAN8 248

/* RedEye protocol definition functions */
const uint8_t redeye_ecc      (uint8_t c);
const uint16_t redeye_frame   (uint8_t c);
void redeye_putc              (const uint8_t c);
void redeye_putesc            (const uint8_t c);

/* LED functions */
int pico_led_init(void);
void pico_set_led(bool led_on);

/* Printer init sequence and static PIO initialisation addresses */
static PIO  pio;
static uint sm;
static uint offset;
static uint8_t initialised = 0x00;
static inline void redeye_printer_init()
{
    if(initialised == 0xFF) return;
    initialised = 0xFF;

    stdio_init_all();
    pico_led_init();
    bool success = pio_claim_free_sm_and_add_program_for_gpio_range(&redeye_program, 
                                                                    &pio,
                                                                    &sm, 
                                                                    &offset, 
                                                                    0, 
                                                                    0,
                                                                    true);
    hard_assert(success);
    redeye_program_init(pio, sm, offset, IR_SIGNAL_PIN);
}

static inline void redeye_printer_shutdown()
{
    if(initialised == 0xFF)
        pio_remove_program_and_unclaim_sm(&redeye_program, pio, sm, offset);
}

#endif /* REDEYE_H */
