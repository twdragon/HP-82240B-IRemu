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
#include "utf8transcoder.h"

#include "config.h"

/* RedEye protocol command table */
#define REDEYE_ESCAPE 27
#define REDEYE_CR     4
#define REDEYE_LF     10
#define REDEYE_FILLER 158 
#define REDEYE_RESET  255
#define REDEYE_TEST   254
#define REDEYE_SETWC  253
#define REDEYE_ENDWC  252
#define REDEYE_SETUL  251
#define REDEYE_ENDUL  250
#define REDEYE_ECMA94 249 // ISO8859-1 [Latin-1]
#define REDEYE_ROMAN8 248

/* RedEye state description variables */
static bool redeye_underline = false;
static bool redeye_wchar = false;
static bool redeye_roman8 = true;
static bool redeye_latin1 = false;

/* RedEye protocol definition functions - forward declarations */
const uint8_t redeye_ecc      (uint8_t c);
const uint16_t redeye_frame   (uint8_t c);

/* Character code substitution routines API */
uint8_t redeye_transcode_latin1(const uint32_t uc);

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
    bool init_success = 
        pio_claim_free_sm_and_add_program_for_gpio_range(&redeye_program, 
                                                         &pio,
                                                         &sm, 
                                                         &offset, 
                                                         0, 
                                                         0,
                                                         true);
    hard_assert(init_success);
    redeye_program_init(pio, sm, offset, IR_SIGNAL_PIN);
    utf8_unpack_tables();
}

static inline void redeye_printer_shutdown()
{
    if(initialised == 0xFF)
        pio_remove_program_and_unclaim_sm(&redeye_program, pio, sm, offset);
}

/* RedEye protocol definition functions - inline low-level */

static inline void redeye_putc(const uint8_t c)
{
    pico_set_led(true);
    pio_sm_put_blocking(pio, sm, (redeye_frame(c) << 16) );
    pico_set_led(false);
}

static inline void redeye_putesc(const uint8_t c)
{
    redeye_putc(REDEYE_ESCAPE);
    redeye_putc(c);
    sleep_ms(1);
}

static inline void redeye_set_underline()
{
    redeye_putesc(REDEYE_SETUL);
    redeye_underline = true;
}

static inline void redeye_stop_underline()
{
    redeye_putesc(REDEYE_ENDUL);
    redeye_underline = false;
}

static inline void redeye_set_wchar()
{
    redeye_putesc(REDEYE_SETWC);
    redeye_wchar = true;
}

static inline void redeye_stop_wchar()
{
    redeye_putesc(REDEYE_ENDWC);
    redeye_wchar = false;
}

static inline void redeye_set_Roman8()
{
    redeye_putesc(REDEYE_ROMAN8);
    redeye_roman8 = true;
    redeye_latin1 = false;
}

static inline void redeye_set_Latin1()
{
    redeye_putesc(REDEYE_ECMA94);
    redeye_roman8 = false;
    redeye_latin1 = true;
}

static inline uint16_t redeye_putln(const char* str, const bool open)
{
    int len = strlen(str);
    uint16_t ldiff = 0;
    int  lprint = len;
    char strbuf[CHAR_LINE_WIDTH];
    if(redeye_wchar)
    {
        if(len > WCHAR_LINE_WIDTH)
        {
            strncpy(strbuf, str, WCHAR_LINE_WIDTH);
            ldiff = (uint16_t)(len - WCHAR_LINE_WIDTH);
            lprint = WCHAR_LINE_WIDTH;
        }
        else
            strncpy(strbuf, str, len);
    }
    else
    {
        if(len > CHAR_LINE_WIDTH)
        {
            strncpy(strbuf, str, CHAR_LINE_WIDTH);
            ldiff = (uint16_t)(len - CHAR_LINE_WIDTH);
            lprint = CHAR_LINE_WIDTH;
        }
        else
            strncpy(strbuf, str, len);
    }
    for(int i = 0; i < lprint; i++)
    {
        redeye_putc((uint8_t)strbuf[i]);
    }
    if(!open)
        redeye_putc(REDEYE_LF);
    sleep_ms(LINEFEED_DURATION);
    return ldiff;
}

#endif /* REDEYE_H */
