#ifndef USB_DESCRIPTORS_H_
#define USB_DESCRIPTORS_H_

enum {
  ITF_NUM_PRINTER = 0,
  ITF_NUM_TOTAL   = 1,
};

// Device identifiers
#define USB_VID   0xCAFE
#define USB_PID   0x4005
#define USB_BCD   0x0200

// Endpoint numbers
#define EPNUM_PRINTER_OUT 0x01
#define EPNUM_PRINTER_IN  0x81

// Descriptor length
#define CONFIG_TOTAL_LEN (TUD_CONFIG_DESC_LEN + TUD_PRINTER_DESC_LEN)

#endif /* USB_DESCRIPTORS_H_ */
