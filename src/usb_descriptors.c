#include "tusb.h"

#include "usb_descriptors.h"

// Device Descriptor
// ============================================================================
uint8_t const *tud_descriptor_device_cb(void) 
{
  return (uint8_t const *) &desc_device;
}

// Configuration Descriptor
// ============================================================================
#if TUD_OPT_HIGH_SPEED
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

// Device ID Descriptor
// ============================================================================
uint8_t const *tud_printer_get_device_id_cb(uint8_t itf)
{
    (void)itf;
    return (uint8_t const *)printer_device_id;
}

// Device Callback Functions
// ============================================================================
void tud_printer_rx_cb(uint8_t itf) 
{
    (void)itf;
}

uint8_t tud_printer_get_port_status_cb(uint8_t itf)
{
    (void)itf;
    tusb_printer_port_status_t status = {0};
    status.status_bm.not_error = redeye_error ? 0 : 1;
    status.status_bm.selected = redeye_busy ? 0 : 1;
    status.status_bm.paper_empty = 0;
    return status.status;
}
