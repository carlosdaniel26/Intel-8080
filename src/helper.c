#include <stdint.h>
#include <stdio.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <errno.h>

#include <helper.h>
#include <cpu.h>

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

Flags byteToFlags(uint8_t byte) 
{
    Flags flags;
    
    flags.cy = READBIT(byte, 0);
    flags.p = READBIT(byte, 2);
    flags.ac = READBIT(byte, 4);
    flags.z = READBIT(byte, 6);
    flags.s = READBIT(byte, 7);

    return flags;
}

uint8_t flagsToByte(Flags flags) 
{
    uint8_t byte = 0;
    
    if (flags.cy) set_bit(byte, 0);
    if (flags.p) set_bit(byte, 2);
    if (flags.ac) set_bit(byte, 4);
    if (flags.z) set_bit(byte, 6);
    if (flags.s) set_bit(byte, 7);
    
    return byte;
}