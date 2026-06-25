#include "redeye.h"

#include "bsp/board_api.h"
#include "tusb.h"
#include "usb_descriptors.h"
#include "usb_interchange.h"

int main(void)
{
#ifdef BUILD_TESTER_APP // Building the tester application instead of the normal printer driver
    redeye_printer_init();
    utf8_unpack_tables();
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
    // Graphics
    redeye_putc(REDEYE_CR);
    redeye_putesc(166);
    for(int i = 0; i < 166; i++)
        redeye_putc(i);
    redeye_putc(REDEYE_CR);
    sleep_ms(LINEFEED_DURATION);
    redeye_putesc(166);
    for(int i = 0; i < 166; i++)
        redeye_putc(255 - 165 + i);
    redeye_putc(REDEYE_CR);
    sleep_ms(LINEFEED_DURATION);
    redeye_putln("100x: ", true);
    redeye_putesc(100);
    for(int i = 0; i < 100; i++)
        redeye_putc(0x08);
    redeye_putc(REDEYE_LF);
    sleep_ms(LINEFEED_DURATION);
    redeye_putln("Graphics: OK", false);
    redeye_putln("========================", false);  
    // Cyrillic
    const char cyrillic_str1[] = "Съешь ещё этих мягких";
    const char cyrillic_str2[] = "французских булок, да";
    const char cyrillic_str3[] = "выпей чаю!";
    redeye_set_Latin1();
    size_t len = strlen(cyrillic_str1);                     // String 1
    uint32_t* decoded = (uint32_t*)malloc(len);
    size_t syslen = utf8_to_utf32(cyrillic_str1, len, decoded);
    for(size_t i = 0; i < syslen; i++)
        redeye_putc(redeye_transcode_latin1(decoded[i]));
    redeye_putc(REDEYE_LF);
    sleep_ms(LINEFEED_DURATION);
    free(decoded);                                          // End of String 1
    len = strlen(cyrillic_str2);                            // String 2
    decoded = (uint32_t*)malloc(len);
    syslen = utf8_to_utf32(cyrillic_str2, len, decoded);
    for(size_t i = 0; i < syslen; i++)
        redeye_putc(redeye_transcode_latin1(decoded[i]));
    redeye_putc(REDEYE_LF);
    sleep_ms(LINEFEED_DURATION);
    free(decoded);                                          // End of String 2
    len = strlen(cyrillic_str3);                            // String 3
    decoded = (uint32_t*)malloc(len);
    syslen = utf8_to_utf32(cyrillic_str3, len, decoded);
    for(size_t i = 0; i < syslen; i++)
        redeye_putc(redeye_transcode_latin1(decoded[i]));
    redeye_putc(REDEYE_LF);
    sleep_ms(LINEFEED_DURATION);
    free(decoded);                                          // End of String 3
    redeye_putln("Cyrillic: OK", false);
    redeye_putc(REDEYE_LF);
#else // BUILD_TESTER_APP
    board_init();
    static tusb_rhport_init_t dev_init = {
        .role = TUSB_ROLE_DEVICE, 
        .speed = TUSB_SPEED_AUTO
    };
    tusb_init(BOARD_TUD_RHPORT, &dev_init);
    board_init_after_tusb();
    redeye_printer_init();
    while(true)
    {
        tud_task();
    }
#endif // BUILD_TESTER_APP
    redeye_printer_shutdown();
    return 0;
}

// IEEE 1284 Device ID: first 2 bytes are big-endian total length
// (including the 2 length bytes).
// ============================================================================
static const char printer_device_id[] =
    "\x00\x7F" 
    "MFG:Hewlett-Packard;"
    "MDL:HP 82240B;"
    "CLS:PRINTER;"
    "CMD:text/plain;charset=utf-8;"
    "DES:UTF-8 text, 166-dot 90-dpi monochrome raster printer;";

TU_VERIFY_STATIC(sizeof(printer_device_id) - 1 == 134, "device ID length mismatch");

void tud_printer_rx_cb(uint8_t itf) 
{
    (void)itf;
}

uint8_t const *tud_printer_get_device_id_cb(uint8_t itf)
{
    (void)itf;
    return (uint8_t const *)printer_device_id;
}