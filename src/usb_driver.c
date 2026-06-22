#include "usb_driver.h"

void usb_init(void)
{
    board_init();
    static tusb_rhport_init_t dev_init = {
        .role = TUSB_ROLE_DEVICE, 
        .speed = TUSB_SPEED_AUTO
    };
    tusb_init(BOARD_TUD_RHPORT, &dev_init);
    board_init_after_tusb();
}

void tud_printer_rx_cb(uint8_t itf) {
  (void)itf;
}

// IEEE 1284 Device ID: first 2 bytes are big-endian total length (including the 2 length bytes).
// The rest is the Device ID string using standard abbreviated keys.
static const char printer_device_id[] =
  "\x00\x7F" // total length = 199
  "MFG:Hewlett-Packard;"
  "MDL:HP 82240B;"
  "CLS:PRINTER;"
  "CMD:text/plain;charset=utf-8;"
  "DES:UTF-8 text, 166-dot monochrome raster printer;";

TU_VERIFY_STATIC(sizeof(printer_device_id) - 1 == 127, "device ID length mismatch");

uint8_t const *tud_printer_get_device_id_cb(uint8_t itf) {
  (void)itf;
  return (uint8_t const *)printer_device_id;
}