#include "redeye.h"
#include "pngle.h"

// PNGle: Logo data and Callback
// ============================================================================
const uint8_t data[309] = {
    0x89, 0x50, 0x4E, 0x47, 0x0D, 0x0A, 0x1A, 0x0A, 0x00, 0x00, 0x00, 0x0D,
    0x49, 0x48, 0x44, 0x52, 0x00, 0x00, 0x00, 0xA6, 0x00, 0x00, 0x00, 0x08,
    0x08, 0x06, 0x00, 0x00, 0x00, 0x73, 0x9E, 0x84, 0xE9, 0x00, 0x00, 0x00, 
    0x01, 0x73, 0x52, 0x47, 0x42, 0x00, 0xAE, 0xCE, 0x1C, 0xE9, 0x00, 0x00, 
    0x00, 0x04, 0x67, 0x41, 0x4D, 0x41, 0x00, 0x00, 0xB1, 0x8F, 0x0B, 0xFC,
    0x61, 0x05, 0x00, 0x00, 0x00, 0x09, 0x70, 0x48, 0x59, 0x73, 0x00, 0x00,
    0x0E, 0xC2, 0x00, 0x00, 0x0E, 0xC2, 0x01, 0x15, 0x28, 0x4A, 0x80, 0x00, 
    0x00, 0x00, 0xCA, 0x49, 0x44, 0x41, 0x54, 0x58, 0x47, 0xED, 0x95, 0x8D, 
    0x0A, 0x84, 0x30, 0x0C, 0x83, 0x37, 0xDF, 0xFF, 0x9D, 0x77, 0x06, 0x0C,
    0x57, 0x4A, 0x5D, 0xAB, 0x37, 0x39, 0x3B, 0xF6, 0xC1, 0x50, 0xB6, 0xD8, 
    0xFD, 0x24, 0xCC, 0x5A, 0x4A, 0x69, 0x7B, 0x4B, 0x41, 0x6B, 0xF6, 0x52, 
    0x6B, 0xC5, 0x36, 0xBE, 0x40, 0x17, 0xED, 0x93, 0xE8, 0x71, 0xC9, 0x5D, 
    0xED, 0x68, 0x1D, 0xB0, 0xB4, 0xD0, 0xE8, 0x7E, 0xAB, 0x2F, 0x0B, 0xDB, 
    0xF1, 0x4C, 0x01, 0x0E, 0x99, 0x07, 0xCD, 0xF7, 0x11, 0x07, 0xCF, 0x3A, 
    0x3A, 0x00, 0x1A, 0x39, 0xE7, 0x99, 0x96, 0x61, 0xB8, 0xA3, 0xB3, 0xB4, 
    0x5A, 0x47, 0xAD, 0xC5, 0x59, 0x7F, 0x46, 0x52, 0x05, 0xF3, 0x69, 0x7A, 
    0xA6, 0x3F, 0x0D, 0x43, 0xF7, 0x0B, 0xFF, 0x5C, 0xFF, 0x68, 0xA6, 0x0E, 
    0x26, 0x4C, 0x1A, 0x69, 0x14, 0xEB, 0xA1, 0xF5, 0x42, 0x24, 0x75, 0x1E, 
    0x52, 0x1B, 0xD1, 0x7B, 0xCC, 0x12, 0xCE, 0x75, 0x63, 0x2E, 0x5E, 0xC9, 
    0x0A, 0xE6, 0x05, 0x78, 0x4B, 0x7A, 0xBF, 0x5C, 0x8C, 0xB3, 0x79, 0x44, 
    0x75, 0x57, 0x40, 0xBD, 0xEC, 0xB7, 0xE6, 0xB4, 0xC1, 0x84, 0x31, 0x34, 
    0x3D, 0x6A, 0x12, 0xBF, 0xE9, 0x91, 0xC5, 0x74, 0x6F, 0x1F, 0x6F, 0x67, 
    0xDA, 0x60, 0x4A, 0x63, 0x3C, 0x93, 0x10, 0xB4, 0x48, 0x28, 0xC9, 0x88, 
    0x70, 0xB2, 0x86, 0x9C, 0xDB, 0x9A, 0x5F, 0xEB, 0xA8, 0x9D, 0x9B, 0x52, 
    0x3E, 0xC7, 0x78, 0x97, 0x27, 0x88, 0x86, 0x07, 0x99, 0x00, 0x00, 0x00, 
    0x00, 0x49, 0x45, 0x4E, 0x44, 0xAE, 0x42, 0x60, 0x82
};
static uint8_t logo_data[166] = {0};
static void on_draw_png_pixel(pngle_t *pngle, 
                              long unsigned int x, 
                              long unsigned int y, 
                              long unsigned int w, 
                              long unsigned int h, 
                              const unsigned char rgba[4])
{
    if(rgba[0] < 0x7F)
        logo_data[x] |= (0x01 << y);
}

// Main entry point
// ============================================================================

int main(void)
{
    redeye_printer_init();
    utf8_unpack_tables();
    sleep_ms(5000);
    redeye_putln("", false);
    redeye_putln("HP 82440B Printer Tester", false);
    redeye_putln("========================", false);
    redeye_putln("FIX WIDTH 24 CHARACTERS", false);
    redeye_putln("========================", false);
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
    redeye_putln("4. Graphics", true);
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
    redeye_putc(REDEYE_CR);
    sleep_ms(LINEFEED_DURATION);
    pngle_t* pngle = pngle_new();
    pngle_set_draw_callback(pngle, on_draw_png_pixel);
    redeye_putesc(166);
    pngle_feed(pngle, png_logo, 309);
    for(int i = 0; i < 166; i++)
        redeye_putc(logo_data[i]);
    pngle_destroy(pngle);
    redeye_putc(REDEYE_LF);
    sleep_ms(LINEFEED_DURATION);
    redeye_putln("Graphics: OK", false);
    redeye_putln("========================", false);  
    // Cyrillic
    redeye_putln("5. Artificial Cyrillic", false);
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
    redeye_putln("===== END OF TESTS =====", false);
    redeye_putln("", false);
    redeye_printer_shutdown();
    return 0;
}
