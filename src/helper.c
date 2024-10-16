#include <stdint.h>

uint16_t twoU8_to_u16adress(uint8_t byte1, uint8_t byte2)
{
    uint8_t lsb = (uint8_t)(byte1 & 0xFF);
    uint8_t msb = (uint8_t)(byte2 & 0xFF);
    
    return (uint16_t)((msb << 8) | lsb);
}

uint16_t twoU8_to_u16value(uint8_t byte1, uint8_t byte2)
{
    return ((byte1 << 8) | byte2);
}