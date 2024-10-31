#ifndef HELPER_H
#define HELPER_H

#include <stdint.h>

#define FLAG_CARRY       0x01
#define FLAG_PARITY      0x04
#define FLAG_AUX_CARRY   0x10
#define FLAG_ZERO        0x40
#define FLAG_SIGN        0x80

#define BIT_0        1
#define BIT_1        2
#define BIT_2        4
#define BIT_3        8
#define BIT_4        16
#define BIT_5        32
#define BIT_6        64
#define BIT_7        128

#define READBIT(A, B) ((A >> (B & 7)) & 1)

#define set_bit(value, bit) value |= bit
#define unset_bit(value, bit) value &= bit 

uint16_t twoU8_to_u16adress(uint8_t byte1, uint8_t byte2);
uint16_t twoU8_to_u16value(uint8_t byte1, uint8_t byte2);

#endif