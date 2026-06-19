#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

int main(int argc, char* argv[]) 
{
    const uint32_t a = 0xC254C301;
    uint8_t* c = (uint8_t*)&a;
    if((c[0] == 0x01) && (c[1] == 0xC3) &&
       (c[2] == 0x54) && (c[3] == 0xC2))
        return 0;
    return 1;
}

