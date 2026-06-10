/*
 * Entry point for the HP 82240B signal-injection test firmware.
 *
 * Execution flow
 * ──────────────
 *  1. Hardware initialisation (clocks, stdio, GPIO, PIO).
 *  2. Short power-on delay — gives the printer MCU time to complete its
 *     own initialisation sequence before we start injecting frames.
 *  3. Printer reset (ESC 0xFF) to put the printer in a known state.
 *  4. Run the full test suite once.
 *  5. Enter interactive mode: commands arrive over USB-CDC (serial monitor).
 *
 * USB Serial Interface
 * ────────────────────
 *  Connect via any terminal program (e.g. minicom, PuTTY, screen) at any
 *  baud rate (USB CDC is baud-agnostic).  Commands are single characters:
 *
 *    '1'  – Test 1: single char 'A'
 *    '2'  – Test 2: ASCII ramp
 *    '3'  – Test 3: linefeed types
 *    '4'  – Test 4: double-wide
 *    '5'  – Test 5: underline
 *    '6'  – Test 6: graphics stripes
 *    '7'  – Test 7: buffer stress
 *    'a'  – Run all tests
 *    'r'  – Send ESC RESET to printer
 *    '?'  – Show this help
 *
 * Power Supply Notes
 * ──────────────────
 *  The printer runs from 4 × AA cells ≈ 6 V (fresh) down to ~4.8 V when
 *  discharged.  The Raspberry Pi Pico accepts 1.8–5.5 V on VSYS and 5 V on
 *  VBUS.  Power the Pico from the printer's battery via the VSYS pin through
 *  a Schottky diode (to prevent back-powering the battery from USB when both
 *  are connected).  The onboard SMPS regulates VSYS down to 3.3 V for the
 *  RP2040.  Do NOT connect the 6 V supply directly to 3V3 or any GPIO pin.
 *
 *  Suggested supply path:
 *      Printer battery (+6 V) → 1N5819 Schottky → Pico VSYS pin
 *      Printer battery (GND)  → Pico GND
 */
#include "pico/stdlib.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "hardware/pio.h"
#include "hardware/gpio.h"

#include "redeye.pio.h"
#include "redeye.h"

#define IR_SIGNAL_PIN    3

static PIO  pio;
static uint sm;
static uint offset;

int main(void)
{
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
    
    while(true)
    {
        // pio_sm_put_blocking(pio, sm, 0x00);
        // const uint32_t command = 0b1100;
        // pio_sm_put_blocking(pio, sm, __rev(command));
        /* for (uint8_t i = 0; i < 10; i++)
        {
            pico_set_led(true);
            pio_sm_put_blocking(pio, sm, (redeye_frame(65 + i) << 16) );
            pico_set_led(false);
            sleep_ms(100);
        }*/
        pico_set_led(true);
        pio_sm_put_blocking(pio, sm, (redeye_frame(65) << 16) );
        pico_set_led(false);
        sleep_ms(300);
    }
    pio_remove_program_and_unclaim_sm(&redeye_program, pio, sm, offset);
    return 0;
}
