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
#include "printbuffer.h"

#include "config.h"
#include "redeye_commands.h"
#include "redeye_state.h"

// RedEye protocol definition functions - forward declarations
// ============================================================================
const uint8_t redeye_ecc      (uint8_t c);
const uint16_t redeye_frame   (uint8_t c);

// LED functions
// ============================================================================
int pico_led_init(void);
void pico_set_led(bool led_on);

// Printer init sequence and static PIO initialisation addresses
// ============================================================================
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
    
    redeye_underline = false;
    redeye_wchar     = false;
    redeye_roman8    = true;
    redeye_latin1    = false;
    redeye_linewrap  = false;
    redeye_error     = false;
    redeye_busy      = false;
}

static inline void redeye_printer_shutdown()
{
    if(initialised == 0xFF)
        pio_remove_program_and_unclaim_sm(&redeye_program, pio, sm, offset);
}

// RedEye protocol definition functions - inline low-level
// ============================================================================
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

static inline void redeye_set_underline(void)
{
    redeye_putesc(REDEYE_SETUL);
    redeye_underline = true;
}

static inline void redeye_stop_underline(void)
{
    redeye_putesc(REDEYE_ENDUL);
    redeye_underline = false;
}

static inline void redeye_set_wchar(void)
{
    redeye_putesc(REDEYE_SETWC);
    redeye_wchar = true;
}

static inline void redeye_stop_wchar(void)
{
    redeye_putesc(REDEYE_ENDWC);
    redeye_wchar = false;
}

static inline void redeye_set_Roman8(void)
{
    redeye_putesc(REDEYE_ROMAN8);
    redeye_roman8 = true;
    redeye_latin1 = false;
}

static inline void redeye_set_Latin1(void)
{
    redeye_putesc(REDEYE_ECMA94);
    redeye_roman8 = false;
    redeye_latin1 = true;
}

// Printing helper functions - inline low-level
// ============================================================================
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
        redeye_putc((uint8_t)strbuf[i]);
    if(!open)
        redeye_putc(REDEYE_LF);
    sleep_ms(LINEFEED_DURATION);
    return ldiff;
}

static inline void redeye_print_buffer(void)
{
    uint8_t c;
    static size_t safety_counter = 0;
    static size_t wrapper_counter = 0;
    size_t last_spacer;
    static bool prev_empty = true;
    static uint8_t line[CHAR_LINE_WIDTH];
    if(buffer_empty)
        return;
    buffer_request_read(&c);
    if(buffer_usage() <= PRINT_BUFFER_LOWWATER)
        redeye_busy = false;
    // Start printing characters
    if(redeye_linewrap) // Word wrapper activated
    {
        bool spacer = false; // Spacer character detector
        switch(c)
        {
        case 0x20: // Natural space
        case 127:  // Filler space
        case 160:  // ISO8859-1 filler space
            spacer = true;
            break;
        default: 
            spacer = false;
            break;
        }
        if(redeye_wchar && spacer) // Wide characters: line wrap on every space
        {
            redeye_putc(REDEYE_LF);
            sleep_ms(LINEFEED_DURATION);
        }
        else // Normal characters: buffered output with REDEYE_LF on demand
        {
            line[wrapper_counter++] = c;
            if(buffer_empty || wrapper_counter >= CHAR_LINE_WIDTH)
            {
                for(last_spacer = wrapper_counter - 1; 
                    last_spacer != 0; 
                    last_spacer--)
                {
                    if((line[last_spacer] == 0x20) || 
                    (line[last_spacer] == 127)  ||
                    (line[last_spacer] == 160))
                        break;
                }
                for(size_t i = 0; i <= last_spacer; i++)
                {
                    redeye_putc(line[i]);
                    if((safety_counter++ > CHAR_LINE_WIDTH) || (line[i] == REDEYE_LF))
                    {
                        sleep_ms(LINEFEED_DURATION);
                        safety_counter = 0;
                    }
                }
                redeye_putc(REDEYE_LF);
                sleep_ms(LINEFEED_DURATION);
                if(last_spacer != wrapper_counter)
                    for(size_t i = last_spacer + 1; i < wrapper_counter; i++)
                    {
                        redeye_putc(line[i]);
                        if((safety_counter++ > CHAR_LINE_WIDTH) || (line[i] == REDEYE_LF))
                        {
                            sleep_ms(LINEFEED_DURATION);
                            safety_counter = 0;
                        }
                    }
                wrapper_counter = 0;
            }
        }
    }
    else // Unbuffered output with safety counter
    {
        redeye_putc(c);
        if((safety_counter++ > CHAR_LINE_WIDTH) || (c == REDEYE_LF))
        {
            sleep_ms(LINEFEED_DURATION);
            safety_counter = 0;
        }
    }
    if(!prev_empty && buffer_empty) // End of print: send REDEYE_LF
    {
        redeye_putc(REDEYE_LF);      // Implicitly ends printing of the last
        sleep_ms(LINEFEED_DURATION); // remaining characters in the string
    }
    prev_empty = buffer_empty;
}

// RedEye command definition functions - forward declarations
// ============================================================================
void redeye_set_word_wrap(void);
void redeye_stop_word_wrap(void);

#endif // REDEYE_H

