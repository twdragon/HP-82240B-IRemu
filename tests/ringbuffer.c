#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stddef.h>

#define BUFFER_SIZE 102400

static uint8_t printbuffer[BUFFER_SIZE] = {0};
static size_t write_index = 0;
static size_t read_index = 0;
static bool buffer_empty = true;
static bool buffer_full = false;

bool buffer_request_write(const uint8_t c)
{
    if((write_index == read_index) && buffer_full && !buffer_empty)
        return false; // Dropping a value, should never be initiated!
    printbuffer[write_index] = c;
    size_t tmp_index = write_index + 1;
    if(tmp_index >= BUFFER_SIZE) // Closing ring
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
    if(tmp_index >= BUFFER_SIZE) // Closing ring
        tmp_index = 0;
    buffer_empty = (tmp_index == write_index); // Initiate EMPTY state
    read_index = tmp_index;
    buffer_full = false;
    return true;
}

int main(int argc, char* argv[]) 
{
    size_t c = 0;
    while(buffer_request_write(0xEF))
        c++;
    if(buffer_empty || !buffer_full || 
       c != (BUFFER_SIZE) || (write_index != 0))
        return 1;
    c = 0;
    uint8_t v;
    while(buffer_request_read(&v))
        c++;
    if(!buffer_empty || buffer_full || 
       c != (BUFFER_SIZE) || (read_index != 0))
        return 1;
    return 0;
}

