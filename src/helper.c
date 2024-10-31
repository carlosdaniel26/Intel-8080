#include <stdint.h>

uint16_t twoU8_to_u16adress(uint8_t msb, uint8_t lsb)
{   
    return (uint16_t)((msb << 8) | lsb);
}

uint16_t twoU8_to_u16value(uint8_t msb, uint8_t lsb)
{
    return ((lsb << 8) | msb);
}