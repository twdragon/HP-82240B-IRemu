#ifndef REDEYE_H
#define REDEYE_H

/*
 * redeye.h
 * ─────────────
 * HP 82240B "RedEye" infrared protocol layer.
 *
 */

#include <stdint.h>
#include <stddef.h>
#include "hardware/pio.h"
#include "pico/bit_ops.h"

/* ── GPIO assignments ───────────────────────────────────────────────────── */
#ifdef CYW43_WL_GPIO_LED_PIN  /* onboard LED, blinks per transmitted line    */
    #include "pico/cyw43_arch.h"
#endif    

/* ── Protocol timing (informational) ───────────────────────────────────── */

#define HP_CARRIER_HZ    32768u   /* nominal carrier frequency                */
#define HP_HALF_BIT_US     427u   /* 427.25 µs per half-bit time            */
#define HP_FRAME_MIN_HBT    30u   /* minimum half-bit times per frame         */

/* ── Control codes ──────────────────────────────────────────────────────── */

#define HP_CODE_ALT_LF   0x04u   /* alternate linefeed: printhead stops RIGHT */
#define HP_CODE_LF       0x0Au   /* ASCII linefeed:     printhead stops LEFT  */
#define HP_CODE_ESC      0x1Bu   /* escape prefix                             */

/* ── Escape sequence second bytes ───────────────────────────────────────── */

#define HP_ESC_RESET         0xFFu
#define HP_ESC_SELFTEST      0xFEu
#define HP_ESC_DBLWIDE       0xFDu
#define HP_ESC_SGLWIDE       0xFCu
#define HP_ESC_UNDERLINE_ON  0xFBu
#define HP_ESC_UNDERLINE_OFF 0xFAu
#define HP_ESC_ISO8859       0xF9u
#define HP_ESC_ROMAN8        0xF8u

int pico_led_init(void);
void pico_set_led(bool led_on);

const uint8_t redeye_ecc      (uint8_t c);    // Calculates RedEye Parity ECC
const uint16_t redeye_frame   (uint8_t c);    // Builds RedEye frame from a character

#endif /* REDEYE_H */