#include <stdint.h>
#include <stdio.h>
#include "cpu.h"

uint16_t twoU8_to_u16value(uint8_t msb, uint8_t lsb)
{   
    return ((msb << 8) | lsb);
}

uint16_t twoU8_to_u16adress(uint8_t msb, uint8_t lsb)
{   
    return twoU8_to_u16value(lsb, msb);
}

uint8_t read_byte(Cpu8080 *cpu)
{
    unsigned PC = cpu->registers.pc;
    return (cpu->rom[PC+1]);
}

uint16_t read_byte_address(Cpu8080 *cpu)
{
    unsigned PC = cpu->registers.pc;
    uint16_t answer = twoU8_to_u16adress(cpu->rom[PC+1], cpu->rom[PC+2]);

    return answer;
}