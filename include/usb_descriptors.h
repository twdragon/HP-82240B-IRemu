#ifndef USB_DESCRIPTORS_H_
#define USB_DESCRIPTORS_H_

#include "bsp/board_api.h"
#include "tusb.h"
#include "usb_descriptors.h"

#include "config.h"
#include "redeye_state.h"
#include "printbuffer.h"
#include "utf8transcoder.h"

// Interface enumerator
// ============================================================================
enum {
  ITF_NUM_PRINTER = 0,
  ITF_NUM_TOTAL   = 1,
};

// Device identifiers
// ============================================================================
#define USB_VID   0x03F0 // HP Inc.
#define USB_PID   0x240B // Artificial, free at 27.06.2026
#define USB_BCD   0x0200 // USB 2.0

// Endpoint numbers
// ============================================================================
#define EPNUM_PRINTER_OUT 0x01
#define EPNUM_PRINTER_IN  0x81

// Descriptor length
// ============================================================================
#define CONFIG_TOTAL_LEN (TUD_CONFIG_DESC_LEN + TUD_PRINTER_DESC_LEN)

// Device Descriptor
// ============================================================================
static tusb_desc_device_t const desc_device = {
  .bLength            = sizeof(tusb_desc_device_t),
  .bDescriptorType    = TUSB_DESC_DEVICE,
  .bcdUSB             = USB_BCD,

  .bDeviceClass       = TUSB_CLASS_PRINTER,
  .bDeviceSubClass    = 0x01,
  .bDeviceProtocol    = 0x02,
  .bMaxPacketSize0    = CFG_TUD_ENDPOINT0_SIZE,

  .idVendor           = USB_VID,
  .idProduct          = USB_PID,
  .bcdDevice          = USB_BCD,

  .iManufacturer      = 0x01,
  .iProduct           = 0x02,
  .iSerialNumber      = 0x03,

  .bNumConfigurations = 0x01
};

uint8_t const *tud_descriptor_device_cb(void);

// Configuration Descriptor
// ============================================================================
#if TUD_OPT_HIGH_SPEED
#define TUSB_PACKET_SIZE 512
#else // TUD_OPT_HIGH_SPEED
#define TUSB_PACKET_SIZE 64
#endif // TUD_OPT_HIGH_SPEED
// Full Speed 
static uint8_t const desc_fs_configuration[] = {
    TUD_CONFIG_DESCRIPTOR(1,                    // Config number
                          ITF_NUM_TOTAL,        // Interface count
                          0,                    // String index
                          CONFIG_TOTAL_LEN,     // Total length
                          0x00,                 // Attributes
                          130),                 // Feeding current, mA
    
    TUD_PRINTER_DESCRIPTOR(ITF_NUM_PRINTER,     // Interface number
                           4,                   // String index
                           EPNUM_PRINTER_OUT,   // EP Bulk Out address
                           EPNUM_PRINTER_IN,    // EP Bulk In address
                           TUSB_PACKET_SIZE)    // EP size
};

#if TUD_OPT_HIGH_SPEED
static uint8_t const desc_hs_configuration[] = {
    TUD_CONFIG_DESCRIPTOR(1, 
                          ITF_NUM_TOTAL, 
                          0, 
                          CONFIG_TOTAL_LEN, 
                          0x00, 
                          100),

    TUD_PRINTER_DESCRIPTOR(ITF_NUM_PRINTER, 
                           4, 
                           EPNUM_PRINTER_OUT, 
                           EPNUM_PRINTER_IN, 
                           TUSB_PACKET_SIZE)
};

static uint8_t desc_other_speed_config[CONFIG_TOTAL_LEN];

uint8_t const *tud_descriptor_other_speed_configuration_cb(uint8_t index);

// Configuration Qualifier (USB High Speed)
// ============================================================================
static tusb_desc_device_qualifier_t const desc_device_qualifier = {
  .bLength            = sizeof(tusb_desc_device_qualifier_t),
  .bDescriptorType    = TUSB_DESC_DEVICE_QUALIFIER,
  .bcdUSB             = USB_BCD,

  .bDeviceClass       = TUSB_CLASS_PRINTER,
  .bDeviceSubClass    = 0x01,
  .bDeviceProtocol    = 0x02,

  .bMaxPacketSize0    = CFG_TUD_ENDPOINT0_SIZE,
  .bNumConfigurations = 0x01,
  .bReserved          = 0x00
};

uint8_t const *tud_descriptor_device_qualifier_cb(void);
#endif // TUD_OPT_HIGH_SPEED

uint8_t const *tud_descriptor_configuration_cb(uint8_t index);

// Info String Descriptor
// ============================================================================
enum {
    STRID_LANGID       = 0,
    STRID_MANUFACTURER = 1,
    STRID_PRODUCT      = 2,
    STRID_SERIAL       = 3,
    STRID_PRINTER      = 4
};

static char const *string_desc_arr[] = {
  (const char[]) { 0x09, 0x04 }, // 0: supported language is English (0x0409)
  "Hewlett-Packard",             // 1: Manufacturer
  "82240B",                      // 2: Product
  PRINTER_SERIAL_NUMBER,         // 3: Serial, use unique ID if possible, 8 ch
  "Thermal 90dpi IrDA Printer",  // 4: Printer Interface
};

static uint16_t _desc_str[32 + 1];

uint16_t const * tud_descriptor_string_cb(uint8_t index, uint16_t langid);

// Device ID Descriptor
// ============================================================================
// IEEE 1284 Device ID: first 2 bytes are big-endian total length
// (including the 2 length bytes).
static const char printer_device_id[] =
    "\x00\x7F" 
    "MFG:HP;"
    "MDL:82240B;"
    "CLS:PRINTER;"
    "CMD:TEXT,UTF-8;"
    "DES:UTF-8 text, 166-dot 90-dpi monochrome raster printer;"
    "SN:"PRINTER_SERIAL_NUMBER";"
    "CID:text/plain,utf-8;";

TU_VERIFY_STATIC(sizeof(printer_device_id) - 1 == 137, 
                 "device ID length mismatch");

uint8_t const *tud_printer_get_device_id_cb(uint8_t itf);

// Device Callback Functions
// ============================================================================
void tud_printer_rx_cb(uint8_t itf);
uint8_t tud_printer_get_port_status_cb(uint8_t itf);

#endif /* USB_DESCRIPTORS_H_ */
