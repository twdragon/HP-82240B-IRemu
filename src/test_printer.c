#include "pico/stdlib.h"
#include "hardware/pio.h"
#include "hardware/gpio.h"
#include "redeye.h"
#include "redeye.pio.h"

int main(void)
{
    redeye_printer_init();
    sleep_ms(5000);
    redeye_putln("", false);
    redeye_putln("HP 82440B Printer Tester", false);
    redeye_putln("==========================", false);
    // Underline
    redeye_putln("1. Underlined", false);
    redeye_set_underline();
    redeye_putln("   TEST STRING   ", false);
    redeye_stop_underline();
    redeye_putln("   TEST STRING   ", true);
    redeye_putln(" OK", false);
    redeye_putln("========================", false);
    // Wide characters
    redeye_putln("2. Wide characters", false);
    redeye_set_wchar();
    redeye_putln(" WIDE character", false);
    redeye_stop_wchar();
    redeye_putln("> WIDE character <", true);
    redeye_putln(" OK", false);
    redeye_putln("========================", false);
    // ISO8859-1 [Latin-1] vs HP Roman8
    redeye_putln("3. HP Roman8 vs", false);
    redeye_putln("   ISO8859-1 [Latin-1]", false);
    int c = 0;
    uint8_t latin_str[CHAR_LINE_WIDTH];
    char num_str[CHAR_LINE_WIDTH];
    int last = 0;
    for(int i = 32; i < 256; i++)
    {
        latin_str[c++] = i;
        if(c >= CHAR_LINE_WIDTH)
        {
            sprintf(num_str, "%03d                  %03d", i - CHAR_LINE_WIDTH + 1, i);
            redeye_putln(num_str, false);
            for(uint8_t j = 0; j < CHAR_LINE_WIDTH; j++)
                redeye_putc(latin_str[j]);
            redeye_putc(REDEYE_LF);
            sleep_ms(LINEFEED_DURATION);
            redeye_set_Latin1();
            for(uint8_t j = 0; j < CHAR_LINE_WIDTH; j++)
                redeye_putc(latin_str[j]);
            redeye_putc(REDEYE_LF);
            sleep_ms(LINEFEED_DURATION);
            redeye_set_Roman8();
            c = 0;
            last = i;
        }
    }
    sprintf(num_str, "%03d                  %03d", last, 255);
    redeye_putln(num_str, false);
    for(uint8_t j = 0; j < c; j++)
        redeye_putc(latin_str[j]);
    redeye_putc(REDEYE_LF);
    sleep_ms(LINEFEED_DURATION);
    redeye_set_Latin1();
    for(uint8_t j = 0; j < c; j++)
        redeye_putc(latin_str[j]);
    sleep_ms(LINEFEED_DURATION);
    redeye_set_Roman8();
    redeye_putln(" OK", false);
    redeye_putln("========================", false);
    // 
    redeye_printer_shutdown();
    return 0;
}
