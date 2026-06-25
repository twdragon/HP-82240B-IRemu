#include "tusb.h"

#include "usb_descriptors.h"

// Device Descriptor
// ============================================================================
static tusb_desc_device_t const desc_device = {
  .bLength            = sizeof(tusb_desc_device_t),
  .bDescriptorType    = TUSB_DESC_DEVICE,
  .bcdUSB             = USB_BCD,

  .bDeviceClass       = TUSB_CLASS_PRINTER,
  .bDeviceSubClass    = MISC_SUBCLASS_COMMON,
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

uint8_t const *tud_descriptor_device_cb(void) 
{
  return (uint8_t const *) &desc_device;
}

// Configuration Descriptor
// ============================================================================
// Full Speed 
static uint8_t const desc_fs_configuration[] = {
    TUD_CONFIG_DESCRIPTOR(1,                    // Config number
                          ITF_NUM_TOTAL,        // Interface count
                          0,                    // String index
                          CONFIG_TOTAL_LEN,     // Total length
                          0x00,                 // Attributes
                          100),                 // Feeding current, mA
    
    TUD_PRINTER_DESCRIPTOR(ITF_NUM_PRINTER,     // Interface number
                           4,                   // String index
                           EPNUM_PRINTER_OUT,   // EP Bulk Out address
                           EPNUM_PRINTER_IN,    // EP Bulk In address
                           64)                  // EP size
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
                           512)
};

static uint8_t desc_other_speed_config[CONFIG_TOTAL_LEN];

uint8_t const *tud_descriptor_other_speed_configuration_cb(uint8_t index) 
{
    (void) index;
    memcpy(desc_other_speed_config,
           (tud_speed_get() == TUSB_SPEED_HIGH) ? 
                desc_fs_configuration : desc_hs_configuration,
           CONFIG_TOTAL_LEN);
    desc_other_speed_config[1] = TUSB_DESC_OTHER_SPEED_CONFIG;
    return desc_other_speed_config;
}

// Configuration Qualifier (USB High Spуed)
// ============================================================================
static tusb_desc_device_qualifier_t const desc_device_qualifier = {
  .bLength            = sizeof(tusb_desc_device_qualifier_t),
  .bDescriptorType    = TUSB_DESC_DEVICE_QUALIFIER,
  .bcdUSB             = USB_BCD,

  .bDeviceClass       = TUSB_CLASS_PRINTER,
  .bDeviceSubClass    = 0x00,
  .bDeviceProtocol    = 0x02,

  .bMaxPacketSize0    = CFG_TUD_ENDPOINT0_SIZE,
  .bNumConfigurations = 0x01,
  .bReserved          = 0x00
};

uint8_t const *tud_descriptor_device_qualifier_cb(void) 
{
  return (uint8_t const *) &desc_device_qualifier;
}
#endif // TUD_OPT_HIGH_SPEED

uint8_t const *tud_descriptor_configuration_cb(uint8_t index) 
{
    (void) index;
#if TUD_OPT_HIGH_SPEED
    return (tud_speed_get() == TUSB_SPEED_HIGH) ? 
        desc_hs_configuration : desc_fs_configuration;
#else
    return desc_fs_configuration;
#endif
}

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
  "30300882",                    // 3: Serial, use unique ID if possible
  "Thermal 90dpi IrDA Printer",  // 4: Printer Interface
};

static uint16_t _desc_str[32 + 1];

uint16_t const * tud_descriptor_string_cb(uint8_t index, uint16_t langid)
{
  (void) langid;

  uint8_t chr_count;

  if (index == 0)
  {
    _desc_str[1] = 0x0409;
    chr_count = 1;
  }
  else
  {
    if (index >= sizeof(string_desc_arr) / sizeof(string_desc_arr[0]))
    {
      return NULL;
    }

    const char *str = string_desc_arr[index];

    chr_count = 0;
    while (str[chr_count] != '\0' && chr_count < 31)
    {
      _desc_str[1 + chr_count] = (uint8_t) str[chr_count];
      chr_count++;
    }
  }

  _desc_str[0] = (uint16_t) ((TUSB_DESC_STRING << 8) | (2 * chr_count + 2));
  return _desc_str;
}
