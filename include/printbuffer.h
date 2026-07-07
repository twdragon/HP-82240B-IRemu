#ifndef PRINTBUFFER_H
#define PRINTBUFFER_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#include "config.h"

extern volatile uint8_t printbuffer[PRINT_BUFFER_SIZE];
extern volatile size_t write_index;
extern volatile size_t read_index;
extern volatile bool buffer_empty;
extern volatile bool buffer_full;

bool buffer_request_write(const uint8_t c);
bool buffer_request_read(uint8_t* c);
static inline void buffer_soft_reset(void)
{
    write_index = 0;
    read_index = 0;
    buffer_empty = true;
    buffer_full = false;
}
void buffer_reset(void);
static inline size_t buffer_usage(void)
{
    return (write_index - read_index + PRINT_BUFFER_SIZE) % PRINT_BUFFER_SIZE;
}

#endif // PRINTBUFFER_H

