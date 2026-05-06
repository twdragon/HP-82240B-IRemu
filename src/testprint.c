#include <stdlib.h>
#include <stdio.h>

#include "pico/stdlib.h"
#include "hardware/pio.h"
#ifdef CYW43_WL_GPIO_LED_PIN
    #include "pico/cyw43_arch.h"
#endif
#include "hardware/gpio.h"
#include "hardware/clocks.h"

#include "ircomm.pio.h"

static const uint OUTPUT_PIN = 5;

int pico_led_init(void) {
#if defined(PICO_DEFAULT_LED_PIN)
    // A device like Pico that uses a GPIO for the LED will define PICO_DEFAULT_LED_PIN
    // so we can use normal GPIO functionality to turn the led on and off
    gpio_init(PICO_DEFAULT_LED_PIN);
    gpio_set_dir(PICO_DEFAULT_LED_PIN, GPIO_OUT);
    return PICO_OK;
#elif defined(CYW43_WL_GPIO_LED_PIN)
    // For Pico W devices we need to initialise the driver etc
    return cyw43_arch_init();
#endif
}

// Turn the led on or off
void pico_set_led(bool led_on) {
#if defined(PICO_DEFAULT_LED_PIN)
    // Just set the GPIO on or off
    gpio_put(PICO_DEFAULT_LED_PIN, led_on);
#elif defined(CYW43_WL_GPIO_LED_PIN)
    // Ask the wifi "driver" to set the GPIO on or off
    cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, led_on);
#endif
}


/* IR Printer Control Sequence */

#define ADD_BLOCKS(val, count) do { \
    for(int i=0; i<(count); i++) { \
        if ((val) & (1<<i)) frame |= (1ULL << pos); \
        pos++; \
    } \
} while(0)

void send_char(PIO pio, uint sm, uint16_t data) {
    uint64_t frame = 0;
    int pos = 0;

    // 1. Start Sequence: 3 bursts. 
    // Burst = LOW (0), HIGH (1). LSB first: 0b101010 = 0x2A
    ADD_BLOCKS(0x2A, 6);

    // 2. Data payload: 12 bits, MSB first
    for (int i = 11; i >= 0; i--) {
        if (data & (1 << i)) {
            // '1' bit: burst first half, silence second half
            // Quantums: LOW, HIGH, HIGH, HIGH -> 0b1110 = 0xE
            ADD_BLOCKS(0xE, 4);
        } else {
            // '0' bit: silence first half, burst second half
            // Quantums: HIGH, HIGH, LOW, HIGH -> 0b1011 = 0xB
            ADD_BLOCKS(0xB, 4);
        }
    }

    // 3. Stop Sequence: At least 3 half-bit times of silence (6 quantums of HIGH)
    ADD_BLOCKS(0x3F, 6);

    // 4. Pad the remainder of the 64-bit frame with 1s to maintain HIGH idle state
    while (pos < 64) {
        frame |= (1ULL << pos);
        pos++;
    }

    // Push the 64-bit frame to the FIFO (lower 32 bits, then upper 32 bits)
    pio_sm_put_blocking(pio, sm, (uint32_t)(frame & 0xFFFFFFFF));
    pio_sm_put_blocking(pio, sm, (uint32_t)(frame >> 32));
}

void print_string(PIO pio, uint sm, const char *str) {
    while (*str) {
        // Send the character with standard 12-bit encoding (upper 4 bits as 0)
        send_char(pio, sm, (uint16_t)(*str));
        str++;
        sleep_ms(15); // Brief delay between characters
    }
    // Send Carriage Return and Line Feed to trigger the print mechanism
    send_char(pio, sm, 0x0D);
    send_char(pio, sm, 0x0A);
}

/* IR Printer Control Sequence */


int main() {
    stdio_init_all();

    if (cyw43_arch_init()) {
        printf("Wi-Fi init failed");
        return -1;
    }

    PIO pio = pio0;
    uint sm =  pio_claim_unused_sm(pio, true);
    uint offset = pio_add_program(pio, &ircomm_program);
    ircomm_program_init(pio, sm, offset, OUTPUT_PIN);

    while (true) {
        pico_set_led(true);
        print_string(pio, sm, "HELLO HP82240 PRINTER");
        sleep_ms(10000);
        printf("LED\n");
        pico_set_led(false);
        sleep_ms(1000);
    }
}

