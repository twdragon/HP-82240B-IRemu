
#include "redeye.h"

int main(void)
{
    redeye_printer_init();
    while(true)
    {
        for (uint8_t i = 0; i < 120; i++)
        {
            pico_set_led(true);
            pio_sm_put_blocking(pio, sm, (redeye_frame(27) << 16) );
            pio_sm_put_blocking(pio, sm, (redeye_frame(1 + i) << 16) );
            pico_set_led(false);
        }
        pio_sm_put_blocking(pio, sm, (redeye_frame(10) << 16) );
        // pico_set_led(true);
        // pio_sm_put_blocking(pio, sm, (redeye_frame(65) << 16) );
        // pico_set_led(false);
        sleep_ms(5000);
    }
    redeye_printer_shutdown();
    return 0;
}
