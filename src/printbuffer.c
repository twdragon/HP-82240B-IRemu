#include "printbuffer.h"

bool buffer_request_write(const uint8_t c)
{
    if((write_index == read_index) && buffer_full && !buffer_empty)
        return false; // Dropping a value, should never be initiated!
    printbuffer[write_index] = c;
    size_t tmp_index = write_index + 1;
    if(tmp_index >= PRINT_BUFFER_SIZE) // Closing ring
        tmp_index = 0;
    buffer_full = (tmp_index == read_index); // Initiate FULL state
    write_index = tmp_index;
    buffer_empty = false;
    return true;
}

bool buffer_request_read(uint8_t* c)
{
    if((read_index == write_index) && !buffer_full && buffer_empty)
        return false;
    (*c) = printbuffer[read_index];
    size_t tmp_index = read_index + 1;
    if(tmp_index >= PRINT_BUFFER_SIZE) // Closing ring
        tmp_index = 0;
    buffer_empty = (tmp_index == write_index); // Initiate EMPTY state
    read_index = tmp_index;
    buffer_full = false;
    return true;
}

