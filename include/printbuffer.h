#ifndef PRINTBUFFER_H
#define PRINTBUFFER_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#include "config.h"
#include "redeye_commands.h"
#include "redeye_state.h"

static uint8_t printbuffer[PRINT_BUFFER_SIZE] = {0};
static size_t write_index = 0;
static size_t read_index = 0;
static bool buffer_empty = true;
static bool buffer_full = false;

bool buffer_request_write(const uint8_t c);
bool buffer_request_read(uint8_t* c);

#endif // PRINTBUFFER_H

