#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>
#include <time.h>
#include <errno.h>
#include <SDL2/SDL.h>

#include "main.h"
#include "cpu.h"
#include "debug.h"
#include "rom.h"

#define FLAG_CARRY       0x01
#define FLAG_PARITY      0x04
#define FLAG_AUX_CARRY   0x10
#define FLAG_ZERO        0x40
#define FLAG_SIGN        0x80

#define BIT_0		 1
#define BIT_1		 2
#define BIT_2	   	 4
#define BIT_3		 8
#define BIT_4		 16
#define BIT_5		 32
#define BIT_6		 64
#define BIT_7		 128

#define READBIT(A, B) ((A >> (B & 7)) & 1)

#define set_bit(value, bit) value |= bit
#define unset_bit(value, bit) value &= bit 

int instruction_cycles[256] = 	{

    [0x00] = 4,  // NOP
    [0x01] = 10, // LXI B, D16
    [0x02] = 7,  // STAX B
    [0x03] = 5,  // INX B
    [0x04] = 4,  // INR B
    [0x05] = 4,  // DCR B
    [0x06] = 7,  // MVI B, D8
    [0x07] = 4,  // RLC
    [0x08] = 4,  // - (Not used/NOP)
    [0x09] = 10, // DAD B
    [0x0A] = 7,  // LDX B
    [0x0B] = 5,  // DCX B
    [0x0C] = 4,  // INR C
    [0x0D] = 4,  // DCR C
    [0x0E] = 7,  // MVI C, D8
    [0x0F] = 4,  // RRC

    [0x10] = 4,  // - (Not used/NOP)
    [0x11] = 10, // LXI D, D16
    [0x12] = 7,  // STAX D
    [0x13] = 5,  // INX D
    [0x14] = 4,  // INR D
    [0x15] = 4,  // DCR D
    [0x16] = 7,  // MVI D, D8
    [0x17] = 4,  // RAL
    [0x18] = 4,  // - (Not used/NOP)
    [0x19] = 10, // DAD D
    [0x1A] = 7,  // LDX D
    [0x1B] = 5,  // DCX D
    [0x1C] = 4,  // INR E
    [0x1D] = 4,  // DCR E
    [0x1E] = 7,  // MVI E, D8
    [0x1F] = 4,  // RAR

    [0x20] = 10, // JM addr
    [0x21] = 10, // LXI H, D16
    [0x22] = 10, // SHLD addr
    [0x23] = 5,  // INX H
    [0x24] = 4,  // INR H
    [0x25] = 4,  // DCR H
    [0x26] = 7,  // MVI H, D8
    [0x27] = 4,  // DAA

    [0x28] = 4,  // - (Not used/NOP)
    [0x29] = 10, // DAD H
    [0x2A] = 10, // LHLD addr
    [0x2B] = 5,  // DCX H
    [0x2C] = 4,  // INR L
    [0x2D] = 4,  // DCR L
    [0x2E] = 7,  // MVI L, D8
    [0x2F] = 4,  // CMA

    [0x30] = 4,  // - (Not used/NOP)
    [0x31] = 10, // LXI SP, D16
    [0x32] = 10, // STA addr
    [0x33] = 5,  // INX SP
    [0x34] = 4,  // INR M
    [0x35] = 4,  // DCR M
    [0x36] = 10, // MVI M, D8
    [0x37] = 4,  // STC

    [0x38] = 4,  // - (Not used/NOP)
    [0x39] = 10, // DAD SP
    [0x3A] = 10, // LDA addr
    [0x3B] = 5,  // DCX SP
    [0x3C] = 4,  // INR A
    [0x3D] = 4,  // DCR A
    [0x3E] = 7,  // MVI A, D8
    [0x3F] = 4,   // CMC

    [0x40] = 4,  // MOV B, B
    [0x41] = 4,  // MOV B, C
    [0x42] = 4,  // MOV B, D
    [0x43] = 4,  // MOV B, E
    [0x44] = 4,  // MOV B, H
    [0x45] = 4,  // MOV B, L
    [0x46] = 7,  // MOV B, M
    [0x47] = 4,  // MOV B, A

    [0x48] = 4,  // MOV C, B
    [0x49] = 4,  // MOV C, C
    [0x4A] = 4,  // MOV C, D
    [0x4B] = 4,  // MOV C, E
    [0x4C] = 4,  // MOV C, H
    [0x4D] = 4,  // MOV C, L
    [0x4E] = 7,  // MOV C, M
    [0x4F] = 4,  // MOV C, A

    [0x50] = 4,  // MOV D, B
    [0x51] = 4,  // MOV D, C
    [0x52] = 4,  // MOV D, D
    [0x53] = 4,  // MOV D, E
    [0x54] = 4,  // MOV D, H
    [0x55] = 4,  // MOV D, L
    [0x56] = 7,  // MOV D, M
    [0x57] = 4,  // MOV D, A

    [0x58] = 4,  // MOV E, B
    [0x59] = 4,  // MOV E, C
    [0x5A] = 4,  // MOV E, D
    [0x5B] = 4,  // MOV E, E
    [0x5C] = 4,  // MOV E, H
    [0x5D] = 4,  // MOV E, L
    [0x5E] = 7,  // MOV E, M
    [0x5F] = 4,  // MOV E, A

    [0x60] = 4,  // MOV H, B
    [0x61] = 4,  // MOV H, C
    [0x62] = 4,  // MOV H, D
    [0x63] = 4,  // MOV H, E
    [0x64] = 4,  // MOV H, H
    [0x65] = 4,  // MOV H, L
    [0x66] = 7,  // MOV H, M
    [0x67] = 4,  // MOV H, A

    [0x68] = 4,  // MOV L, B
    [0x69] = 4,  // MOV L, C
    [0x6A] = 4,  // MOV L, D
    [0x6B] = 4,  // MOV L, E
    [0x6C] = 4,  // MOV L, H
    [0x6D] = 4,  // MOV L, L
    [0x6E] = 7,  // MOV L, M
    [0x6F] = 4,  // MOV L, A
    
    [0x70] = 7,  // MOV M, B
    [0x71] = 7,  // MOV M, C
    [0x72] = 7,  // MOV M, D
    [0x73] = 7,  // MOV M, E
    [0x74] = 7,  // MOV M, H
    [0x75] = 7,  // MOV M, L
    [0x76] = 4,  // HLT
    [0x77] = 7,  // MOV M, A

    [0x78] = 4,  // MOV A, B
    [0x79] = 4,  // MOV A, C
    [0x7A] = 4,  // MOV A, D
    [0x7B] = 4,  // MOV A, E
    [0x7C] = 4,  // MOV A, H
    [0x7D] = 4,  // MOV A, L
    [0x7E] = 7,  // MOV A, M
    [0x7F] = 4,  // MOV A, A

    [0x80] = 4,  // ADD B
    [0x81] = 4,  // ADD C
    [0x82] = 4,  // ADD D
    [0x83] = 4,  // ADD E
    [0x84] = 4,  // ADD H
    [0x85] = 4,  // ADD L
    [0x86] = 7,  // ADD M
    [0x87] = 4,  // ADD A

    [0x88] = 4,  // ADC B
    [0x89] = 4,  // ADC C
    [0x8A] = 4,  // ADC D
    [0x8B] = 4,  // ADC E
    [0x8C] = 4,  // ADC H
    [0x8D] = 4,  // ADC L
    [0x8E] = 7,  // ADC M
    [0x8F] = 4,  // ADC A

    [0x90] = 4,  // SUB B
    [0x91] = 4,  // SUB C
    [0x92] = 4,  // SUB D
    [0x93] = 4,  // SUB E
    [0x94] = 4,  // SUB H
    [0x95] = 4,  // SUB L
    [0x96] = 7,  // SUB M
    [0x97] = 4,  // SUB A

    [0x98] = 4,  // SBB B
    [0x99] = 4,  // SBB C
    [0x9A] = 4,  // SBB D
    [0x9B] = 4,  // SBB E
    [0x9C] = 4,  // SBB H
    [0x9D] = 4,  // SBB L
    [0x9E] = 7,  // SBB M
    [0x9F] = 4,  // SBB A
    
    [0xA0] = 4,  // ANA B
    [0xA1] = 4,  // ANA C
    [0xA2] = 4,  // ANA D
    [0xA3] = 4,  // ANA E
    [0xA4] = 4,  // ANA H
    [0xA5] = 4,  // ANA L
    [0xA6] = 7,  // ANA M
    [0xA7] = 4,  // ANA A

    [0xA8] = 4,  // XRA B
    [0xA9] = 4,  // XRA C
    [0xAA] = 4,  // XRA D
    [0xAB] = 4,  // XRA E
    [0xAC] = 4,  // XRA H
    [0xAD] = 4,  // XRA L
    [0xAE] = 7,  // XRA M
    [0xAF] = 4,  // XRA A

    [0xB0] = 4,  // ORA B
    [0xB1] = 4,  // ORA C
    [0xB2] = 4,  // ORA D
    [0xB3] = 4,  // ORA E
    [0xB4] = 4,  // ORA H
    [0xB5] = 4,  // ORA L
    [0xB6] = 7,  // ORA M
    [0xB7] = 4,  // ORA A

    [0xB8] = 4,  // RIM
    [0xB9] = 4,  // SIM
    [0xBA] = 4,  // - (Not used/NOP)
    [0xBB] = 4,  // - (Not used/NOP)
    [0xBC] = 4,  // - (Not used/NOP)
    [0xBD] = 4,  // - (Not used/NOP)
    [0xBE] = 4,  // - (Not used/NOP)
    [0xBF] = 4,   // - (Not used/NOP)

    [0xC0] = 5,  // RZ
    [0xC1] = 10, // POP B
    [0xC2] = 10, // JP addr
    [0xC3] = 10, // JMP addr
    [0xC4] = 11, // CALL addr
    [0xC5] = 10, // PUSH B
    [0xC6] = 7,  // ADI D8
    [0xC7] = 11, // RST 0

    [0xC8] = 5,  // RNC
    [0xC9] = 10, // RET
    [0xCA] = 10, // JP C
    [0xCB] = 4,  // - (Not used/NOP)
    [0xCC] = 11, // CALL C
    [0xCD] = 17, // CALL addr
    [0xCE] = 7,  // ACI D8
    [0xCF] = 11, // RST 1

    [0xD0] = 5,  // RPO
    [0xD1] = 10, // POP D
    [0xD2] = 10, // JP PO
    [0xD3] = 7,  // OUT D8
    [0xD4] = 11, // CALL PO
    [0xD5] = 10, // PUSH D
    [0xD6] = 7,  // SUI D8
    [0xD7] = 11, // RST 2

    [0xD8] = 5,  // RPE
    [0xD9] = 10, // JP PE
    [0xDA] = 10, // JP P
    [0xDB] = 7,  // IN D8
    [0xDC] = 11, // CALL PE
    [0xDD] = 10, // - (Not used/NOP)
    [0xDE] = 7,  // SBI D8
    [0xDF] = 11, // RST 3

    [0xE0] = 5,  // RPO
    [0xE1] = 10, // POP H
    [0xE2] = 10, // JP P
    [0xE3] = 10, // XTHL
    [0xE4] = 11, // CALL P
    [0xE5] = 10, // PUSH H
    [0xE6] = 7,  // ANI D8
    [0xE7] = 11, // RST 4

    [0xE8] = 5,  // RPE
    [0xE9] = 10, // JP PE
    [0xEA] = 10, // JP E
    [0xEB] = 10, // XCHG
    [0xEC] = 11, // CALL E
    [0xED] = 10, // - (Not used/NOP)
    [0xEE] = 7,  // XRI D8
    [0xEF] = 11, // RST 5

    [0xF0] = 5,  // RP
    [0xF1] = 10, // POP PSW
    [0xF2] = 10, // JP P
    [0xF3] = 4,  // DI
    [0xF4] = 11, // CALL P
    [0xF5] = 10, // PUSH PSW
    [0xF6] = 7,  // ORI D8
    [0xF7] = 11, // RST 6

    [0xF8] = 5,  // RM
    [0xF9] = 10, // SPHL
    [0xFA] = 10, // JP M
    [0xFB] = 4,  // EI
    [0xFC] = 11, // CALL M
    [0xFD] = 10, // - (Not used/NOP)
    [0xFE] = 7,  // CPI D8
    [0xFF] = 11  // RST 7

};

Cpu8080 cpu;
unsigned int rom_size;

// Acesse os registros diretamente
uint8_t* A;
uint8_t* B;
uint8_t* C;
uint8_t* D;
uint8_t* E;
uint8_t* H;
uint8_t* L;


void video_buffer_to_screen(Cpu8080 *cpu);
void update_screen();

void init_cpu() 
{

    memset(&cpu.registers, 0, sizeof(cpu.registers)); 

    cpu.memory = (uint8_t*)calloc(TOTAL_MEMORY_SIZE, sizeof(uint8_t));
    memset(cpu.memory + VIDEO_RAM_START, 0, VIDEO_RAM_SIZE);
    if (! cpu.memory) {
        fprintf(stderr, "Erro na alocação de memória para o CPU: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }

    A = &cpu.registers.A;
    B = &cpu.registers.B;
    C = &cpu.registers.C;
    D = &cpu.registers.D;
    E = &cpu.registers.E;
    H = &cpu.registers.H;
    L = &cpu.registers.L;

    cpu.registers.sp = 32;

    printf("CPU initialized\n");
}

void copy_rom_to_ram(Cpu8080* cpu, unsigned int rom_size)
{
	char *rom = (char*)&cpu->rom;

	unsigned int i = 0;

	for(i=0; i < rom_size; i++)
	{
		cpu->memory[i] = rom[i];
	}

}


uint8_t MachineIN(uint8_t port)
{
    return port;
	// uint8_t a;

	// switch(port)
	// 	{
	//	uint16_t v = ();
	//	}
}

void set_flag(Cpu8080 *cpu, uint16_t result16, uint8_t value, uint8_t carry)
{
    uint8_t result8 = result16 & 0xFF;

    cpu->registers.F &= ~FLAG_ZERO;
    if (result8 == 0) {
        cpu->registers.F |= FLAG_ZERO;
    }

    cpu->registers.F &= ~FLAG_SIGN;
    if (result8 & 0x80) {
        cpu->registers.F |= FLAG_SIGN;
    }

    cpu->registers.F &= ~FLAG_PARITY;
    if (!__builtin_parity(result8)) {
        cpu->registers.F |= FLAG_PARITY;
    }

    cpu->registers.F &= ~FLAG_CARRY;
    if (result16 > 0xFF) {
        cpu->registers.F |= FLAG_CARRY;
    }

    cpu->registers.F &= ~FLAG_AUX_CARRY;
    if (((cpu->registers.A & 0x0F) + (value & 0x0F) + carry) > 0x0F) {
        cpu->registers.F |= FLAG_AUX_CARRY;
    }
}

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

void NOP() 
{
    cpu.registers.pc++;
    return;
}

// Load register pair immediate
void LXI(Cpu8080 *cpu, uint8_t *reg_low, uint8_t *reg_high) 
{
    *reg_low = cpu->rom[cpu->registers.pc + 2];
    *reg_high = cpu->rom[cpu->registers.pc + 1];
    cpu->registers.pc += 3;
}

void LXI_16(Cpu8080 *cpu, uint16_t *reg) 
{
    uint8_t lsb = (uint8_t)(cpu->rom[cpu->registers.pc+1] & 0xFF);
    uint8_t msb = (uint8_t)(cpu->rom[cpu->registers.pc+2] & 0xFF);
    
    uint16_t address = twoU8_to_u16adress(lsb, msb);

    *reg = address;
    
    cpu->registers.pc += 3;
}

void LDA(Cpu8080 *cpu, uint16_t adress)
{
    cpu->registers.A = cpu->memory[adress];

    cpu->registers.pc += 3;
}

void LDAX(Cpu8080 *cpu, uint8_t *_register1, uint8_t *_register2)
{
    uint8_t  lsb = (uint8_t)((*_register1) & 0xFF);
    uint8_t  msb = (uint8_t)((*_register2) & 0xFF);

    uint8_t address = twoU8_to_u16adress(msb, lsb);

    cpu->registers.A = cpu->memory[address];

    cpu->registers.pc++;
}

void STAX(Cpu8080 *cpu, uint8_t *_register1, uint8_t *_register2)
{
    uint16_t adress = twoU8_to_u16adress(*_register1, *_register2);
    cpu->memory[adress] = cpu->registers.A;
    cpu->registers.pc++;
}

void STA(Cpu8080 *cpu, uint8_t value)
{
    cpu->registers.A = value;
    cpu->registers.pc++;
}

void STC(Cpu8080 *cpu)
{
    cpu->registers.F |= FLAG_CARRY;
    cpu->registers.pc++;
}

void MOV_reg_to_reg(uint8_t *target, uint8_t *source)
{
    *target = *source;
    cpu.registers.pc++;
}

void MOV_mem_to_reg(Cpu8080 *cpu, uint8_t *target)
{
    uint16_t adress = twoU8_to_u16adress(cpu->registers.H, cpu->registers.L);

    *target = cpu->memory[adress];

    cpu->registers.pc += 3;
}

void MOV_reg_to_mem(Cpu8080 *cpu, uint8_t *source)
{
    uint16_t adress = twoU8_to_u16adress(cpu->registers.H, cpu->registers.L);

    cpu->memory[adress] = *source;

    cpu->registers.pc += 3;
}

void MOV_im_to_reg(Cpu8080 *cpu, uint8_t *target)
{
    uint8_t value = cpu->memory[cpu->registers.pc + 1];
    *target = value;
    cpu->registers.pc += 2;
}

void MOV_im_to_mem(Cpu8080 *cpu)
{
    uint16_t address = twoU8_to_u16adress(cpu->registers.H, cpu->registers.L);
    uint8_t value = cpu->memory[cpu->registers.pc+=1];

    cpu->memory[address] = value;

    cpu->registers.pc += 2;
}

void ADD(Cpu8080 *cpu, uint8_t *_register)
{
    cpu->registers.A += *_register;

    cpu->registers.pc++;
}

void ADC(Cpu8080 *cpu, uint8_t *_register)
{
    uint16_t result16 = *_register + cpu->registers.A + (cpu->registers.F & 1);
    uint8_t result8 = result16 & 0xFF;

    set_flag(cpu, result16, *_register, cpu->registers.F & 1);

    cpu->registers.A = result8;

    cpu->registers.pc++;
}

void ACI(Cpu8080 *cpu)
{
    uint8_t value = cpu->rom[cpu->registers.pc];
    uint16_t temp = value + cpu->registers.A + (cpu->registers.F & 1);

    set_flag(cpu, temp, value, cpu->registers.F & 1);

    cpu->registers.A = temp & 0xFF;

    cpu->registers.pc +=1;
}

void SBI(Cpu8080 *cpu)
{
    uint8_t value = cpu->rom[cpu->registers.pc];
    uint16_t temp = value - cpu->registers.A - (cpu->registers.F & 1);

    set_flag(cpu, temp, value, cpu->registers.F & 1);

    cpu->registers.A = temp & 0xFF;

    cpu->registers.pc +=1;
}

void SUI(Cpu8080 *cpu, uint8_t value)
{
    uint16_t result16 = cpu->registers.A - value;
    uint8_t result8 = result16 & 0xFF;

    set_flag(cpu, result16, value, 0);

    cpu->registers.A = result8;

    cpu->registers.pc++;
}

void SUB(Cpu8080 *cpu, uint8_t *_register)
{
    uint16_t result16 = cpu->registers.A - *_register;
    uint8_t result8 = result16 & 0xFF;

    set_flag(cpu, result16, *_register, 0);

    cpu->registers.A = result8;

    cpu->registers.pc++;
}

void SBB(Cpu8080 *cpu, uint8_t *_register)
{
    uint16_t result16 = cpu->registers.A - (*_register - (cpu->registers.F & 1));
    uint8_t result8 = result16 & 0xFF;

    set_flag(cpu, result16, *_register, cpu->registers.F & 1);

    cpu->registers.A = result8;

    cpu->registers.pc++;
}

void ANA(Cpu8080 *cpu, uint8_t *_register)
{
    cpu->registers.A = cpu->registers.A & *_register;
    set_flag(cpu, cpu->registers.A, 0, 0);

    cpu->registers.pc++;
}

void ANI(Cpu8080 *cpu)
{
	uint8_t value = cpu->rom[cpu->registers.pc];

    cpu->registers.A = cpu->registers.A & value;

    set_flag(cpu, cpu->registers.A, 0, 0);

    cpu->registers.pc++;
}

void XRA(Cpu8080 *cpu, uint8_t *_register) 
{
    cpu->registers.A ^= *_register;
    set_flag(cpu, cpu->registers.A, 0, 0);

    cpu->registers.pc++;
}


void ORA(Cpu8080 *cpu, uint8_t *_register)
{
    cpu->registers.A = cpu->registers.A | *_register;
    set_flag(cpu, cpu->registers.A, 0, 0);

    cpu->registers.pc++;
}

void XRI(Cpu8080 *cpu, uint8_t value)
{
    cpu->registers.A = cpu->registers.A ^ value;
    set_flag(cpu, cpu->registers.A, 0, 0);

    cpu->registers.pc++;
}

void CMP(Cpu8080 *cpu, uint8_t *_register) 
{
    uint16_t result16 = cpu->registers.A - *_register;
    set_flag(cpu, result16, *_register, 0);

    cpu->registers.pc++;
}

void DCX(uint8_t *_register1, uint8_t *_register2) 
{
    uint16_t byte_combined = twoU8_to_u16value(*_register1, *_register2);
    byte_combined -= 1;

    *_register1 = (uint8_t)(byte_combined >> 8);
    *_register2 = (uint8_t)(byte_combined & 0xFF);

    cpu.registers.pc++;
}

void DCX_16( uint16_t *_register)
{
    (*_register)--;

    cpu.registers.pc++;
}

void DCR(Cpu8080 *cpu, uint8_t *_register)
{
    (*_register)-=1;
    set_flag(cpu, *_register, 0, 0);

    cpu->registers.pc++;
}

void INX(uint8_t *_register1, uint8_t *_register2) 
{
    uint16_t byte_combined = twoU8_to_u16value(*_register1, *_register2);
    byte_combined -= 1;

    *_register1 = (uint8_t)(byte_combined >> 8);
    *_register2 = (uint8_t)(byte_combined & 0xFF);

    cpu.registers.pc++;
}

void INX_16(uint16_t *_register)
{
    (*_register)++;

    cpu.registers.pc++;
}

void INR(Cpu8080 *cpu, uint8_t *_register)
{
    (*_register)++;
    set_flag(cpu, *_register, 0, 0);

    cpu->registers.pc++;
}

void INR_16(uint16_t *_register)
{
    (*_register)++;

    cpu.registers.pc++;
}


void LHLD(Cpu8080 *cpu)
{
    unsigned int *pc = &cpu->registers.pc;
    uint16_t     adress = twoU8_to_u16adress(cpu->rom[(*pc)+1], cpu->rom[(*pc)+2]);

    cpu->registers.L = cpu->memory[adress];
    cpu->registers.H = cpu->memory[adress + 1];

    pc+=2;
}

void CPI(Cpu8080 *cpu, uint8_t value)
{
    uint16_t result16 = cpu->registers.A - value;
    set_flag(cpu, result16, 0, 0);

    cpu->registers.pc+=2;
}

void CMA(Cpu8080 *cpu)
{
    cpu->registers.A = ~cpu->registers.A;
    cpu->registers.pc+=1;
}

void DAD(Cpu8080 *cpu, uint8_t *_register1, uint8_t *_register2) 
{
    uint8_t H = cpu->registers.H;
    uint8_t L = cpu->registers.L;

    uint16_t byte_combined = twoU8_to_u16value(*_register1, *_register2);
    uint16_t HL = twoU8_to_u16value(H, L);

    HL += byte_combined;

    cpu->registers.H = (uint8_t)(HL >> 8);
    cpu->registers.L = (uint8_t)(HL & 0xFF);
    cpu->registers.pc+=1;

}

void DAD_16(Cpu8080 *cpu, uint16_t *_register) 
{
    uint8_t H = cpu->registers.H;
    uint8_t L = cpu->registers.L;

    uint16_t HL = twoU8_to_u16value(H, L);

    HL += *_register;

    cpu->registers.H = (uint8_t)(HL >> 8);
    cpu->registers.L = (uint8_t)(HL & 0xFF);

    cpu->registers.pc+=2;

}

void ADI(Cpu8080 *cpu)
{
	uint8_t *A   = &cpu->registers.A;
    uint8_t *ROM = (uint8_t*)&cpu->rom;
    unsigned int *PC  = &cpu->registers.pc;

    uint8_t value = ROM[(*PC) + 1];
    
	*A += value;
	set_flag(cpu, *A, 0, 0);

	cpu->registers.pc+=2;

}

void RLC(Cpu8080 *cpu)
{
    uint8_t bit7 = cpu->registers.A & BIT_7;
    cpu->registers.A <<= 1;
    if (bit7) {
        cpu->registers.A |= BIT_0;
        cpu->registers.F |= FLAG_CARRY;
    } else {
        cpu->registers.F &= ~FLAG_CARRY;
    }

    cpu->registers.pc+=1;
}

void RRC(Cpu8080 *cpu)
{
    uint8_t *A = &cpu->registers.A;
    uint8_t *F = &cpu->registers.F;

    // get 7th bit
    uint8_t prev_bit_7 = *A & ~BIT_7;
    uint8_t prev_bit_0 = *A & ~BIT_0;

    *A = *A >> 1;

    // bit[7] = prev_bit[0]
    *A |= prev_bit_0;

    // set CY bit with bit[0]
    *F |= prev_bit_7;

    cpu->registers.pc+=1;    
}

void RAL(Cpu8080 *cpu)
{
    uint8_t prev_carry = cpu->registers.F & FLAG_CARRY;
    uint8_t prev_bit_7 = cpu->registers.A & BIT_7;

    cpu->registers.A = cpu->registers.A << 1;
    
    cpu->registers.A &= ~prev_carry; 

    cpu->registers.F &= ~prev_bit_7;

    cpu->registers.pc+=1;

}

void RAR(Cpu8080 *cpu)
{
    uint8_t *A = &cpu->registers.A;
    uint8_t *F = &cpu->registers.F; 

    uint8_t prev_bit_7 = *A & ~BIT_7;
    uint8_t prev_bit_0 = *A & ~BIT_0;

    *A = *A >> 1;

    // bit[7] = prev_bit[7]
    *A = *A & ~prev_bit_7;

    // CY = bit[0]
    *F = ~prev_bit_0;

    cpu->registers.pc+=1;
    
}

void SHLD(Cpu8080 *cpu)
{
    uint16_t adress = twoU8_to_u16adress(cpu->registers.H, cpu->registers.L);

    cpu->memory[adress] = cpu->registers.L;
    cpu->memory[adress+1] = cpu->registers.H;

    cpu->registers.pc+=3;
}

void CMC(Cpu8080 *cpu)
{
    uint8_t *F = &cpu->registers.F;

    //get CARRY bit
    uint8_t CARRY =  *F & FLAG_CARRY;

    *F ^= CARRY;

    cpu->registers.pc+=1;
}

void POP(Cpu8080 *cpu, uint8_t *register_1, uint8_t *register_2)
{
    uint16_t sp = cpu->registers.sp;
    *register_2 = cpu->memory[sp++];
    *register_1 = cpu->memory[sp++];
    cpu->registers.sp = sp;

    cpu->registers.pc+=1;
}



void PUSH(Cpu8080 *cpu, uint8_t *register_1, uint8_t *register_2)
{
    uint16_t sp = cpu->registers.sp;
    cpu->memory[--sp] = *register_1;
    cpu->memory[--sp] = *register_2;
    cpu->registers.sp = sp;

    cpu->registers.pc+=1;
}

void JC(Cpu8080 *cpu)
{
    
    uint8_t *F = &cpu->registers.F;
    uint8_t *ROM = (uint8_t*)&cpu->rom;
     unsigned int *PC = &cpu->registers.pc;

    uint8_t adress_to_pc = ROM[(*PC) + 1];

    uint8_t CY = READBIT(*F, FLAG_CARRY);


    if (CY == 1)
		*PC = adress_to_pc;
    else
        (*PC)+=3;
}



void JNC(Cpu8080 *cpu)
{
    
    uint8_t *F = &cpu->registers.F;
    uint8_t *ROM = (uint8_t*)&cpu->rom;
    unsigned int *PC = &cpu->registers.pc;

    uint8_t adress_to_pc = ROM[(*PC) + 1];

    uint8_t CY = READBIT(*F, FLAG_CARRY);


    if (CY != 1)
		*PC = adress_to_pc;
    else
        (*PC) += 3;
}

void JP(Cpu8080 *cpu)
{
    uint8_t *ROM = (uint8_t*)&cpu->rom;
     unsigned int *PC  = &cpu->registers.pc;
    uint8_t adress_low = ROM[(*PC)+1];
    uint8_t adress_high = ROM[(*PC) + 2];
    uint16_t adress_pc = twoU8_to_u16adress(adress_low, adress_high);
    
    uint8_t *F = &cpu->registers.F;

    // if Parity bit is true, then
    if (READBIT(*F, FLAG_PARITY))
	   *PC = adress_pc;
    else
        (*PC) += 3;
}

void JPO (Cpu8080 *cpu)
{
    uint8_t *ROM = (uint8_t*)&cpu->rom;
     unsigned int *PC  = &cpu->registers.pc;
    uint8_t adress_low = ROM[(*PC)+1];
    uint8_t adress_high = ROM[(*PC) + 2];
    uint16_t adress_pc = twoU8_to_u16adress(adress_low, adress_high);
    
    uint8_t *F = &cpu->registers.F;

    // if Sign bit is true, then
    if (READBIT(*F, FLAG_SIGN))
    	*PC = adress_pc;
    else
        (*PC) += 3;    
}

void JM (Cpu8080 *cpu)
{
    uint8_t *ROM = (uint8_t*)&cpu->rom;
     unsigned int *PC  = &cpu->registers.pc;
    uint8_t adress_low = ROM[(*PC)+1];
    uint8_t adress_high = ROM[(*PC) + 2];
    uint16_t adress_pc = twoU8_to_u16adress(adress_low, adress_high);
    
    uint8_t *F = &cpu->registers.F;

    // if Sign bit is false, then
    if (! (READBIT(*F, FLAG_SIGN)))
	   *PC = adress_pc;
    else
        (*PC) += 3;    
}

void JNZ (Cpu8080 *cpu)
{

    uint8_t *ROM = (uint8_t*)&cpu->rom;
     unsigned int *PC  = &cpu->registers.pc;
    uint8_t adress_low = ROM[(*PC)+1];
    uint8_t adress_high = ROM[(*PC) + 2];
    uint16_t adress_pc = twoU8_to_u16adress(adress_low, adress_high);
    
    uint8_t *F = &cpu->registers.F;

    // if ZERO bit is false, then
    if (! (READBIT(*F, FLAG_ZERO)))
		*PC = adress_pc;
    else
        (*PC) += 3;      
}

void JZ (Cpu8080 *cpu)
{

    uint8_t *ROM = (uint8_t*)&cpu->rom;
     unsigned int *PC  = &cpu->registers.pc;
    uint8_t adress_low = ROM[(*PC)+1];
    uint8_t adress_high = ROM[(*PC) + 2];
    uint16_t adress_pc = twoU8_to_u16adress(adress_low, adress_high);
    
    uint8_t *F = &cpu->registers.F;

    // if ZERO bit is true, then
    if ((READBIT(*F, FLAG_ZERO)) == 0)
		*PC = adress_pc;
    else 
        (*PC) += 3;      
}

void JMP(Cpu8080 *cpu)
{
    unsigned int *PC  = &cpu->registers.pc;
    uint8_t lsb = (uint8_t)(cpu->rom[cpu->registers.pc+1] & 0xFF);
    uint8_t msb = (uint8_t)(cpu->rom[cpu->registers.pc+2] & 0xFF);
    
    uint16_t adress = twoU8_to_u16adress(lsb, msb);

    *PC = adress;
}

void CP(Cpu8080 *cpu)
{
    uint8_t *ROM = (uint8_t*)&cpu->rom;
    unsigned int *PC  = &cpu->registers.pc;
    uint8_t adress_low = ROM[*PC+1];
    uint8_t adress_high = ROM[(*PC) + 2];
    uint16_t adress_pc = twoU8_to_u16adress(adress_low, adress_high);
    
    uint8_t *F = &cpu->registers.F;

    // if Parity bit is true, then
    if (READBIT(*F, FLAG_PARITY))
	   *PC = adress_pc;
    else
        (*PC) += 3;
}

void XCHG(Cpu8080 *cpu)
{
    uint8_t prev_H = cpu->registers.H;
    uint8_t prev_L = cpu->registers.L;

    cpu->registers.H = cpu->registers.D;
    cpu->registers.L = cpu->registers.E;

    cpu->registers.D = prev_H;
    cpu->registers.E = prev_L; 

    cpu->registers.pc+=1;
}

void SPHL(Cpu8080 *cpu)
{
	uint16_t HL = twoU8_to_u16value(cpu->registers.H, cpu->registers.L);
	uint16_t *SP = &cpu->registers.sp;

	*SP = HL;

    cpu->registers.pc+=1;
}

void PCHL(Cpu8080 *cpu)
{
	unsigned int *PC = &cpu->registers.pc;
	uint16_t HL = twoU8_to_u16value(cpu->registers.H, cpu->registers.L);
			 
	*PC = (unsigned int)HL; 

    cpu->registers.pc+=1;
}

void XTHL(Cpu8080 *cpu)
{
	uint8_t  *H = &cpu->registers.L;
	uint8_t  *L = &cpu->registers.H;
	uint16_t *SP = &cpu->registers.sp;

	uint8_t prev_H = *H;
	uint8_t prev_L = *L;

	*L = cpu->memory[*SP];
	cpu->memory[*SP] = prev_L;

	*H = cpu->memory[*SP+1];
	cpu->memory[*SP+1] = prev_H;

    cpu->registers.pc+=1;
}

void ORI(Cpu8080 *cpu)
{
	uint8_t *A = &cpu->registers.A;
	uint8_t *ROM = (uint8_t*)&cpu->rom;
    unsigned int *PC  = &cpu->registers.pc;
    uint8_t data = ROM[(*PC) + 1];
	
	*A |= data;

	set_flag(cpu, *A, 0, 0);
	(*PC)+=2;
}

void RST(Cpu8080* cpu, unsigned int new_pc_position)
{
	cpu->registers.pc = new_pc_position;
}

void CALL(Cpu8080 *cpu, unsigned int adress)
{
	unsigned int *PC = &cpu->registers.pc;
	uint16_t     *SP = &cpu->registers.sp;
	uint8_t		 *memory = (uint8_t*)&cpu->memory;
	
	unsigned int Higher = *PC >> 8;
	unsigned int Lower = *PC & 0xFF;	

	
	memory[*SP-2] = Higher;
	memory[*SP-1] = Lower;

	SP-=2;
	*PC = adress; 
}

void CALL_adr(Cpu8080 *cpu)
{
    uint8_t lsb = (uint8_t)(cpu->rom[cpu->registers.pc+1] & 0xFF);
    uint8_t msb = (uint8_t)(cpu->rom[cpu->registers.pc+2] & 0xFF);
    
	unsigned int adress = (unsigned int)twoU8_to_u16value(msb, lsb);
	print_debug_message("adress:%x", adress);
	CALL(cpu, adress);	
}

void CM (Cpu8080 *cpu)
{
    uint8_t *ROM = (uint8_t*)&cpu->rom;
    unsigned int *PC  = &cpu->registers.pc;
    uint8_t adress_low = ROM[(*PC)+1];
    uint8_t adress_high = ROM[(*PC) + 2];
    unsigned int adress_pc = (unsigned int)twoU8_to_u16adress(adress_low, adress_high);
    
    uint8_t *F = &cpu->registers.F;

    // if Sign bit is false, then
    if (! (READBIT(*F, FLAG_SIGN)))
    {
		CALL(cpu, adress_pc);
    }

    (*PC) += 2;    
}

void CZ (Cpu8080 *cpu)
{
    uint8_t *ROM = (uint8_t*)&cpu->rom;
     unsigned int *PC  = &cpu->registers.pc;
    uint8_t adress_low = ROM[(*PC)+1];
    uint8_t adress_high = ROM[(*PC) + 2];
    unsigned int adress_pc = (unsigned int)twoU8_to_u16adress(adress_low, adress_high);
    
    uint8_t *F = &cpu->registers.F;

    // if Zero bit is true, then
    if (READBIT(*F, FLAG_ZERO))
    {
		CALL(cpu, adress_pc);
    }

    (*PC) += 2;    
}

void CNZ (Cpu8080 *cpu)
{
    uint8_t *ROM = (uint8_t*)&cpu->rom;
     unsigned int *PC  = &cpu->registers.pc;
    uint8_t adress_low = ROM[(*PC)+1];
    uint8_t adress_high = ROM[(*PC) + 2];
    unsigned int adress_pc = (unsigned int)twoU8_to_u16adress(adress_low, adress_high);
    
    uint8_t *F = &cpu->registers.F;

    // if Zero bit is false, then
    if (! (READBIT(*F, FLAG_ZERO)))
    {
		CALL(cpu, adress_pc);
    }

    (*PC) += 2;    
}

void CC (Cpu8080 *cpu)
{
    uint8_t *ROM = (uint8_t*)&cpu->rom;
    unsigned int *PC  = &cpu->registers.pc;
    uint8_t adress_low = ROM[(*PC)+1];
    uint8_t adress_high = ROM[(*PC) + 2];
    unsigned int adress_pc = (unsigned int)twoU8_to_u16adress(adress_low, adress_high);
    
    uint8_t *F = &cpu->registers.F;

    // if Carry bit is true, then
    if (READBIT(*F, FLAG_CARRY))
    {
		CALL(cpu, adress_pc);
    }

    (*PC) += 2;    
}

void CNC (Cpu8080 *cpu)
{
    uint8_t      *ROM           = (uint8_t*)&cpu->rom;
    unsigned int *PC            = &cpu->registers.pc;
    uint8_t      adress_low     = ROM[*PC];
    uint8_t      adress_high    = ROM[(*PC) + 1];
    unsigned int adress_pc      = (unsigned int)twoU8_to_u16adress(adress_low, adress_high);
    
    uint8_t *F = &cpu->registers.F;

    // if Carry bit is false, then
    if (! (READBIT(*F, FLAG_CARRY)))
    {
		CALL(cpu, adress_pc);
    }

    (*PC) += 2;    
}

void RET(Cpu8080 *cpu)
{
    uint8_t       *memory = cpu->memory;
    uint16_t *SP = &cpu->registers.sp;
	unsigned int  Higher  = memory[*SP+1];
	unsigned int  Lower   = memory[*SP];	

	SP+=2;
	
	cpu->registers.pc = twoU8_to_u16adress(Higher, Lower); 
}

void RZ (Cpu8080 *cpu)
{ 
    uint8_t *F = &cpu->registers.F;

    // if Zero bit is true, then
    if (READBIT(*F, FLAG_ZERO))
		RET(cpu);
    else
        cpu->registers.pc += 1;
}

void RNZ (Cpu8080 *cpu)
{
    uint8_t *F = &cpu->registers.F;

    // if Zero bit is false, then
    if (! (*F & FLAG_ZERO))
		RET(cpu);
    else
        cpu->registers.pc += 1;    
}

void RNC (Cpu8080 *cpu)
{ 
    uint8_t *F = &cpu->registers.F;

    // if Carry bit is false, then
    if (! (READBIT(*F, FLAG_CARRY)))
		RET(cpu);
    else
        cpu->registers.pc += 1;   
}

void RC (Cpu8080 *cpu)
{   
    uint8_t *F = &cpu->registers.F;

    // if Carry bit is true, then
    if (READBIT(*F, FLAG_CARRY))
		RET(cpu);
    else
        cpu->registers.pc += 1;    
}

void RP (Cpu8080 *cpu)
{   
    uint8_t *F = &cpu->registers.F;

    // if Parity bit is true, then
    if (*F & FLAG_PARITY)
		RET(cpu);
    else
        cpu->registers.pc += 1;    
}

void RM (Cpu8080 *cpu)
{    
    uint8_t *F = &cpu->registers.F;

    // if Parity bit is false, then
    if (! (READBIT(*F, FLAG_PARITY)))
		RET(cpu);
    else
        cpu->registers.pc += 1;    
}

void EI(Cpu8080* cpu)
{
	cpu->interrupt_enabled = 1;
    cpu->registers.pc+=1;
}

void DI(Cpu8080* cpu)
{
	cpu->interrupt_enabled = 0;
    cpu->registers.pc+=1;
}

void HLT()
{
	exit(1);
}

void IN(Cpu8080* cpu)
{
	// uint8_t port = cpu->rom[((*PC) + 1)];
	
	// get input like a getch
	// cpu->registers.A = machineIN() 
	
	cpu->registers.pc++;
}

void OUT(Cpu8080 *cpu)
{
	// unsigned int *PC = &cpu->registers.pc;
	// uint8_t port = cpu->rom[((*PC) + 1)];
	
	// MachineOUT()
	cpu->registers.pc++;	
}

void load_rom()
{
    cpu.rom = get_rom();
}

void load_rom_to_memory(Cpu8080 *cpu) 
{
    size_t rom_size = get_rom_size();
    for (size_t i = 0; i <  rom_size; i++)
    {
        cpu->memory[i] = cpu->rom[i];
    }
}

int get_current_instruction_cycles_number()
{
    return instruction_cycles[cpu.registers.pc];
}

void emulate_instruction(Cpu8080 *cpu) 
{
    if (cpu->rom == NULL) {
        fprintf(stderr, "Failed to load ROM\n");
        exit(1);
        return;
    }

	// copy_rom_to_ram(&cpu, rom_size);

    start_clock_debug(cpu);
    char c = getchar();
    if (c == 'q')
        exit(0);

    uint8_t instruction = cpu->rom[cpu->registers.pc];

    switch (instruction) {
        case 0x00: case 0x08: case 0x10: case 0x20: case 0x30:
            NOP();
            break;

        case 0x01:
            LXI(cpu, &cpu->registers.B, &cpu->registers.C);
            break;

        case 0x02:
            STAX(cpu, &cpu->registers.B, &cpu->registers.C);
            break;

        case 0x03:
            INX( &cpu->registers.B, &cpu->registers.C);
            break;

        case 0x04:
            INR(cpu, &cpu->registers.B);
            break;

        case 0x05:
            DCR(cpu, &cpu->registers.B);
            break;

        case 0x06:
            MOV_im_to_reg(cpu, &cpu->registers.B);
            break;

	    case 0x07:
			RLC(cpu);
			break;

        case 0x09:
            DAD(cpu, &cpu->registers.B, &cpu->registers.C);
            break;

        case 0x0A:
            LDAX(cpu, &cpu->registers.B, &cpu->registers.C);
            break;

        case 0x0B:
            DCX(&cpu->registers.B, &cpu->registers.C);
            break;

        case 0x0C:
            INR(cpu, &cpu->registers.C);
            break;

		case 0x0D:
			DCR(cpu, &cpu->registers.C);
			break;

        case 0x0E:
            MOV_im_to_reg(cpu, &cpu->registers.C);
            break;
    
		 case 0x0F:
			RRC(cpu);
			break;
	
        case 0x11:
            LXI(cpu, &cpu->registers.D, &cpu->registers.E);
            break;

        case 0x12:
            STAX(cpu, &cpu->registers.D, &cpu->registers.E);
            break;

        case 0x13:
            INX( &cpu->registers.D, &cpu->registers.E);
            break;

        case 0x14:
            INR(cpu, &cpu->registers.D);
            break;

        case 0x15:
            DCR(cpu, &cpu->registers.D);
            break;

        case 0x16:
            MOV_im_to_reg(cpu, &cpu->registers.D);
            break;
    
		case 0x017:
			RAL(cpu);		
			break;

        case 0x19:
            DAD(cpu, &cpu->registers.D, &cpu->registers.E);
            break;

        case 0x1A:
            LDAX(cpu, &cpu->registers.D, &cpu->registers.E);
            break;

        case 0x1B:
            DCX(&cpu->registers.D, &cpu->registers.E);
            break;

        case 0x1C:
            INR(cpu, &cpu->registers.E);
            break;

        case 0x1D:
            DCR(cpu, &cpu->registers.E);
            break;

        case 0x1E:
            MOV_im_to_reg(cpu, &cpu->registers.E);
            break;

		case 0x1F:
			RAR(cpu);
			break;

		case 0x21:
            LXI(cpu, &cpu->registers.H, &cpu->registers.L);
            break;

		case 0x22:
			SHLD(cpu);
			break;
        
		case 0x23:
			INX( &cpu->registers.H, &cpu->registers.L);
            break;

        case 0x24:
            INR(cpu, &cpu->registers.H);
            break;

        case 0x25:
            DCR(cpu, &cpu->registers.H);
            break;

        case 0x26:
            MOV_im_to_reg(cpu, &cpu->registers.H);
            break;

        case 0x29:
            DAD(cpu, &cpu->registers.H, &cpu->registers.L);
            break;

        case 0x2a:
            LHLD(cpu);
            break;

        case 0x2b:
            DCX(&cpu->registers.H, &cpu->registers.L);
            break;

        case 0x2c:
            INR(cpu, &cpu->registers.L);
            break;

        case 0x2d:
            DCR(cpu, &cpu->registers.L);
            break;

        case 0x2e:
            MOV_im_to_reg(cpu, &cpu->registers.L);
            break;

        case 0x2f:
            CMA(cpu);
            break;


        case 0x31:
            LXI_16(cpu, (uint16_t*)&cpu->registers.sp);
            break;

        case 0x32:
            STA(cpu, cpu->rom[cpu->registers.pc+1]);
            break;

        case 0x33:
            INX_16( &cpu->registers.sp);
            break;

        case 0x34:
            {
                uint8_t adress = twoU8_to_u16value(cpu->registers.H, cpu->registers.L);
                INR(cpu, &cpu->memory[adress]);
                cpu->registers.pc+=2;
            }
            break;

        case 0x35:
                DCR(cpu, &cpu->registers.A);
            break;

        case 0x36:
            MOV_im_to_mem(cpu);
            break;

        case 0x37:
            STC(cpu);
            break;

        case 0x39:
            DAD_16(cpu, &cpu->registers.sp);
            break;

        case 0x3A:
            {
                uint8_t adress = twoU8_to_u16value(cpu->registers.H, cpu->registers.L);
                LDA(cpu, adress);
                cpu->registers.pc+=2;
            }
            break;

        case 0x3B:
            DCX_16(&cpu->registers.sp);
            break;

        case 0x3C:
            INR(cpu, &cpu->registers.A);
            break;

        case 0x3D:
            DCR(cpu, &cpu->registers.A);
            break;

        case 0x3E:
            MOV_im_to_reg(cpu, &cpu->registers.A);
            break;

		case 0x3F:
			CMC(cpu);
			break;

        // MOVs
        case 0x40:
            MOV_reg_to_reg(B, B);
            break;

        case 0x41:
            MOV_reg_to_reg(B, C);
            break;

        case 0x42:
            MOV_reg_to_reg(B, D);
            break;

        case 0x43:
            MOV_reg_to_reg(B, E);
            break;

        case 0x44:
            MOV_reg_to_reg(B, H);
            break;

        case 0x45:
            MOV_reg_to_reg(B, L);
            break;

        case 0x46:
            MOV_mem_to_reg(cpu, B);
            break;

        case 0x47:
            MOV_reg_to_reg(B, A);
            break;

        case 0x48:
            MOV_reg_to_reg(C, B);
            break;

        case 0x49:
            MOV_reg_to_reg(C, C);
            break;

        case 0x4A:
            MOV_reg_to_reg(C, D);
            break;

        case 0x4B:
            MOV_reg_to_reg(C, E);
            break;

        case 0x4C:
            MOV_reg_to_reg(C, H);
            break;

        case 0x4D:
            MOV_reg_to_reg(C, L);
            break;

        case 0x4E:
            MOV_mem_to_reg(cpu, C);
            break;

        case 0x4F:
            MOV_reg_to_reg(C, A);
            break;

        case 0x50:
            MOV_reg_to_reg(D, B);
            break;

        case 0x51:
            MOV_reg_to_reg(D, C);
            break;

        case 0x52:
            MOV_reg_to_reg(D, D);
            break;

        case 0x53:
            MOV_reg_to_reg(D, E);
            break;

        case 0x54:
            MOV_reg_to_reg(D, H);
            break;

        case 0x55:
            MOV_reg_to_reg(D, L);
            break;

        case 0x56:
            MOV_mem_to_reg(cpu, D);
            break;

        case 0x57:
            MOV_reg_to_reg(D, A);
            break;

        case 0x58:
            MOV_reg_to_reg(E, B);
            break;

        case 0x59:
            MOV_reg_to_reg(E, C);
            break;

        case 0x5A:
            MOV_reg_to_reg(E, D);
            break;

        case 0x5B:
            MOV_reg_to_reg(E, E);
            break;

        case 0x5C:
            MOV_reg_to_reg(E, H);
            break;

        case 0x5D:
            MOV_reg_to_reg(E, L);
            break;

        case 0x5E:
            MOV_mem_to_reg(cpu, E);
            break;

        case 0x5F:
            MOV_reg_to_reg(E, A);
            break;

        case 0x60:
            MOV_reg_to_reg(H, B);
            break;

        case 0x61:
            MOV_reg_to_reg(H, C);
            break;

        case 0x62:
            MOV_reg_to_reg(H, D);
            break;

        case 0x63:
            MOV_reg_to_reg(H, E);
            break;

        case 0x64:
            MOV_reg_to_reg(H, H);
            break;

        case 0x65:
            MOV_reg_to_reg(H, L);
            break;

        case 0x66:
            MOV_mem_to_reg(cpu, H);
            break;

        case 0x67:
            MOV_reg_to_reg(H, A);
            break;

        case 0x68:
            MOV_reg_to_reg(L, B);
            break;

        case 0x69:
            MOV_reg_to_reg(L, C);
            break;

        case 0x6A:
            MOV_reg_to_reg(L, D);
            break;

        case 0x6B:
            MOV_reg_to_reg(L, E);
            break;

        case 0x6C:
            MOV_reg_to_reg(L, H);
            break;

        case 0x6D:
            MOV_reg_to_reg(L, L);
            break;

        case 0x6E:
            MOV_mem_to_reg(cpu, L);
            break;

        case 0x6F:
            MOV_reg_to_reg(L, A);
            break;

        case 0x70:
            MOV_reg_to_mem(cpu, B);
            break;

        case 0x71:
            MOV_reg_to_mem(cpu, C);
            break;

        case 0x72:
            MOV_reg_to_mem(cpu, D);
            break;

        case 0x73:
            MOV_reg_to_mem(cpu, E);
            break;

        case 0x74:
            MOV_reg_to_mem(cpu, H);
            break;

        case 0x75:
            MOV_reg_to_mem(cpu, L);
            break;

        case 0x76:
            HLT();
            break;

        case 0x77:
            MOV_reg_to_mem(cpu, A);
            break;

        case 0x78:
            MOV_reg_to_reg(A, B);
            break;

        case 0x79:
            MOV_reg_to_reg(A, C);
            break;

        case 0x7a:
            MOV_reg_to_reg(A, D);
            break;

        case 0x7b:
            MOV_reg_to_reg(A, E);
            break;

        case 0x7c:
            MOV_reg_to_reg(A, H);
            break;

        case 0x7d:
            MOV_reg_to_reg(A, L);
            break;

        case 0x7e:
            MOV_mem_to_reg(cpu, A);
            break;

        case 0x7f:
            MOV_reg_to_reg(A, A);
            break;

        // ADDs
        case 0x80:
            ADD(cpu, B);
            break;

        case 0x81:
            ADD(cpu, C);
            break;

        case 0x82:
            ADD(cpu,D);
            break;

        case 0x83:
            ADD(cpu, E);
            break;

        case 0x84:
            ADD(cpu, H);
            break;

        case 0x85:
            ADD(cpu, L);
            break;

        case 0x86:
            {
                uint16_t mem_adress = twoU8_to_u16adress(cpu->registers.H, cpu->registers.L); 
                uint8_t value = cpu->memory[mem_adress];

                ADD(cpu, &value);
                cpu->registers.pc+=2;
            }
            break;

        case 0x87:
            ADD(cpu, A);
            break;

        // ADCs
        case 0x88:
            ADC(cpu, B);
            break;

        case 0x89:
            ADC(cpu, C);
            break;

        case 0x8a:
            ADC(cpu, D);
            break;

        case 0x8b:
            ADC(cpu, E);
            break;

        case 0x8c:
            ADC(cpu, H);
            break;

        case 0x8d:
            ADC(cpu, L);
            break;

        case 0x8e:
            {
                uint16_t mem_adress = twoU8_to_u16adress(cpu->registers.H, cpu->registers.L); 
                uint8_t value = cpu->memory[mem_adress];

                ADC(cpu, &value);
                cpu->registers.pc+=2;
            }
            break;

        case 0x8f:
            ADC(cpu, A);
            break;

        // SUBs
        case 0x90:
            SUB(cpu, B);
            break;

        case 0x91:
            SUB(cpu, C);
            break;

        case 0x92:
            SUB(cpu, D);
            break;

        case 0x93:
            SUB(cpu, E);
            break;

        case 0x94:
            SUB(cpu, H);
            break;

        case 0x95:
            SUB(cpu, L);
            break;

        case 0x96:
            {
                uint16_t mem_adress = twoU8_to_u16adress(cpu->registers.H, cpu->registers.L); 
                uint8_t value = cpu->memory[mem_adress];

                SUB(cpu, &value);

                cpu->registers.pc+=2;
            }
            break;

        case 0x97:
            SUB(cpu, A);
            break;


        // SBBs
        case 0x98:
            SBB(cpu, B);
            break;

        case 0x99:
            SBB(cpu, C);
            break;


        case 0x9a:
            SBB(cpu, D);
            break;

        case 0x9b:
            SBB(cpu, E);
            break;

        case 0x9c:
            SBB(cpu, H);
            break;

        case 0x9d:
            SBB(cpu, L);
            break;

        case 0x9e:
            {
                uint16_t mem_adress = twoU8_to_u16adress(cpu->registers.H, cpu->registers.L); 
                uint8_t value = cpu->memory[mem_adress];

                SBB(cpu, &value);
                cpu->registers.pc+=2;
            }
            break;

        case 0x9f:
            SBB(cpu, A);
            break;

        // ANAs
        case 0xa0:
            ANA(cpu, B);
            break;

        case 0xa1:
            ANA(cpu, C);
            break;

        case 0xa2:
            ANA(cpu, D);
            break;

        case 0xa3:
            ANA(cpu, E);
            break;

        case 0xa4:
            ANA(cpu, H);
            break;

        case 0xa5:
            ANA(cpu, L);
            break;

        case 0xa6:
            {
                uint16_t mem_adress = twoU8_to_u16adress(cpu->registers.H, cpu->registers.L); 
                uint8_t value = cpu->memory[mem_adress];

                ANA(cpu, &value);
                cpu->registers.pc+=2;
            }
            break;


        // XRAs
        case 0xA8:
            XRA(cpu, B);
            break;

        case 0xA9:
            XRA(cpu, C);
            break;

        case 0xAA:
            XRA(cpu, D);
            break;

        case 0xAB:
            XRA(cpu, E);
            break;

        case 0xAC:
            XRA(cpu, H);
            break;

        case 0xAD:
            XRA(cpu, L);
            break;

        case 0xAE:
            {
                uint16_t mem_adress = twoU8_to_u16adress(cpu->registers.H, cpu->registers.L); 
                uint8_t value = cpu->memory[mem_adress];

                XRA(cpu, &value);
                cpu->registers.pc+=2;
            }
            break;

        case 0xAF:
            XRA(cpu, A);
            break;

        // ORAs
        case 0xB0:
            ORA(cpu, &cpu->registers.B);
            break;

        case 0xB1:
            ORA(cpu, &cpu->registers.C);
            break;

        case 0xB2:
            ORA(cpu, &cpu->registers.D);
            break;

        case 0xB3:
            ORA(cpu, &cpu->registers.E);
            break;

        case 0xB4:
            ORA(cpu, &cpu->registers.H);
            break;

        case 0xB5:
            ORA(cpu, &cpu->registers.L);
            break;

        case 0xB6:
            {
                uint16_t mem_adress = twoU8_to_u16adress(cpu->registers.H, cpu->registers.L); 
                uint8_t value = cpu->memory[mem_adress];

                ORA(cpu, &value);
                cpu->registers.pc+=2;
            }
            break;

        case 0xB7:
            ORA(cpu, &cpu->registers.A);
            break;

        // CMPs
        case 0xB8:
            CMP(cpu, &cpu->registers.B);
            break;

        case 0xB9:
            CMP(cpu, &cpu->registers.C);
            break;

        case 0xBA:
            CMP(cpu, &cpu->registers.D);
            break;

        case 0xBB:
            CMP(cpu, &cpu->registers.E);
            break;

        case 0xBC:
            CMP(cpu, &cpu->registers.H);
            break;

        case 0xBD:
            CMP(cpu, &cpu->registers.L);
            break;

        case 0xBE:
                {
                    uint16_t mem_adress = twoU8_to_u16adress(cpu->registers.H, cpu->registers.L); 
                    uint8_t value = cpu->memory[mem_adress];

                    CMP(cpu, &value);
                    cpu->registers.pc+=2;
                }
                break;

    	case 0xBF:
            CMP(cpu, &cpu->registers.A);
    		break;

    	case 0xC0:
    		RNZ(cpu);
    		break;

    	case 0xC1:
    		POP(cpu, &cpu->registers.B, &cpu->registers.C);
    		break;

    	case 0xC2:
    		JNZ(cpu);
    		break;

    	case 0xC3:
    		JMP(cpu);	
    		break;

    	case 0xC4:
    		CNZ(cpu);
    		break;

    	case 0xC5:
    		PUSH(cpu, &cpu->registers.B, &cpu->registers.C);
    		break;
       
        case 0xC6:
    		ADI(cpu);
    		break;
    	
    	case 0xC7:
    		RST(cpu, 0x00);
    		break;

    	case 0xC8:
    		RZ(cpu);
    		break;

    	case 0xC9:
    		RET(cpu);
    		break;

    	case 0xCA:
    		JZ(cpu);
    		break;

    	case 0xCC:
    		CZ(cpu);
    		break;
    	
    	case 0xCD: 
    		CALL_adr(cpu);
    		break;
    	
    	case 0xCE:
    		ACI(cpu);	
    		break;

    	case 0xCF:
    		RST(cpu, 0x08);
    		break;

    	case 0xD0:
    		RNC(cpu);
    		break;

        case 0xD1:
    		POP(cpu, &cpu->registers.D, &cpu->registers.E);
    		break;

        case 0xD2:
    		JNC(cpu);
    		break;
    	
    	case 0xD3:
    		OUT(cpu);
    		break;

    	case 0xD4:
    		CNC(cpu);
    		break;

        case 0xD6:
            SUI(cpu, cpu->rom[cpu->registers.pc+1]);
            break;

    	case 0xD7:
    		RST(cpu, 0x10);
    		break;

    	case 0xD8:
    		RC(cpu);
    		break;

    	case 0xDA:
    		JC(cpu);
    		break;

    	case 0xDB:
    		IN(cpu);
    		break;
    	
    	case 0xDC:
    		CC(cpu);
    		break;
    	
    	case 0xDE:
    		SBI(cpu);
    		break;

    	case 0xDF:
    		RST(cpu, 0x18);
    		break;
        
        case 0xE1:
    		POP(cpu, &cpu->registers.H, &cpu->registers.L);
    		break;
        
        case 0xE2:
    		JPO(cpu);
    		break;
    	
    	case 0xE3:
    		XTHL(cpu);
    		break;

        case 0xE5:
    		PUSH(cpu, &cpu->registers.H, &cpu->registers.L);
    		break;
    	
    	case 0xE6:
    		ANI(cpu);
    		break;

    	case 0xE7:
    		RST(cpu, 0x20);
    		break;
    	
    	case 0xE9:
    		PCHL(cpu);
    		break;

        case 0xEB:	
    		XCHG(cpu);
    		break;
    	
    	case 0xEE:
            {
                uint16_t mem_adress = twoU8_to_u16adress(cpu->registers.H, cpu->registers.L); 
                uint8_t value = cpu->memory[mem_adress];

                XRI(cpu, value);
                cpu->registers.pc+=2;
            }
            break;

    	case 0xEF:
    		RST(cpu, 0x28);
    		break;
    	
    	case 0xF0:
    		RP(cpu);
    		break;

    	case 0xF1:
    		POP(cpu, &cpu->registers.A, &cpu->registers.F);
    		break;

        case 0xF2:
    		JP(cpu);
    		break;

    	case 0xF3:
    		DI(cpu);
    		break;	

        case 0xF4:
    		CP(cpu);
    		break;
    	
    	case 0xF5:
    		PUSH(cpu, &cpu->registers.A, &cpu->registers.F);	
    		break;
    	
    	case 0xF6:
    		ORI(cpu);
    		break;
    	
    	case 0xF7:
    		RST(cpu, 0x30);
    		break;

    	case 0xF8:
    		RM(cpu);
    		break;
    	
    	case 0xF9:
    		SPHL(cpu);
    		break;
    	
    	case 0xFA:
    		JM(cpu);
    		break;
    	
    	case 0xFB:
    		EI(cpu);
    		break;	

    	case 0xFC:
    		CM(cpu);
    		break;

        case 0xFE:
            {
                uint16_t mem_adress = twoU8_to_u16adress(cpu->registers.H, cpu->registers.L); 
                uint8_t value = cpu->memory[mem_adress];

                CPI(cpu, value);
            }
            break;

    	case 0xFF:
    		RST(cpu, 0x38);
    		break;

        default:
    		printf("Unimplemented instruction: 0x%02X\n", instruction);
            cpu->registers.pc++;
            break;
    }

    update_clock_debug(cpu);
}

SDL_Window *window;
SDL_Renderer *renderer;
SDL_Texture *texture;
Uint32 screen_buffer[VIDEO_RAM_SIZE * 8];

void init_sdl()
{
    if (SDL_Init(SDL_INIT_VIDEO) < 0)
    {
        fprintf(stderr, "Não foi possível inicializar SDL: %s\n", SDL_GetError());
        exit(1);
    }
}

void create_window()
{
    window = SDL_CreateWindow(
        "Intel 8080",
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        WIDTH, HEIGHT,
        SDL_WINDOW_SHOWN
    );

    if (! window)
    {
        fprintf(stderr, "Não foi possível criar a janela: %s\n", SDL_GetError());
        SDL_Quit();
		exit(1);
    }

}

void create_render()
{
    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

    if (! renderer)
    {
        fprintf(stderr, "An error ocurred while trying to create the renderer: %s\n", SDL_GetError());
        SDL_DestroyWindow(window);
        SDL_Quit();
        exit(1);
    }

}

void create_texture()
{
    /* Cria uma textura para o buffer de vídeo */
    texture = SDL_CreateTexture(
        renderer,
        SDL_PIXELFORMAT_RGBA8888,
        SDL_TEXTUREACCESS_STREAMING,
        WIDTH,
        HEIGHT
    );

    if (! texture)
    {
        fprintf(stderr, "An error ocurred while trying to create the texture: %s\n", SDL_GetError());
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        SDL_Quit();
        exit(1);
    }	
}

void init_sdl_screen_buffer()
{
    SDL_PixelFormat *format = SDL_AllocFormat(SDL_PIXELFORMAT_RGBA8888);

    /* Initialize in black screen */
    for (int index = 0; index < (WIDTH * HEIGHT); index++)
    {
        screen_buffer[index] = SDL_MapRGBA(format, 255, 0, 0, 255); // Red if the videobuffer be filled worng
    }
}

void init_video_buffer(Cpu8080 *cpu)
{
    for (unsigned i = VIDEO_RAM_START; i < (VIDEO_RAM_END); i++)
    {
        cpu->memory[VIDEO_RAM_START] = 0xFF;
    }
}

void update_screen()
{
	/* Update the texture with videobuffer */
    SDL_UpdateTexture(texture, NULL, screen_buffer, (WIDTH) * sizeof(Uint32));
    /* Clean screen(renderer) */
    SDL_RenderClear(renderer);
    /* Draw the texture on the renderer */
    SDL_RenderCopy(renderer, texture, NULL, NULL);
    /* Update screen with new renderer */ 
	SDL_RenderPresent(renderer);

    /* Delay for the cpu */
    //SDL_Delay(16);  /* Aprox 60 FPS */

}

void finish_and_free()
{
    /* Free memory and close SDL */
    SDL_DestroyTexture(texture);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

	free(cpu.rom);
    free(cpu.memory);

}

void intel8080_main()
{
    init_video_buffer(&cpu);

    /* Main */
    SDL_Event event;
    int running = 1;  /* Flag to control loop execution       */
    while (running)
    {
        /* Get events from SDL */
        while (SDL_PollEvent(&event))
        {
            /* IF event is to close window, then stop to run */
            if (event.type == SDL_QUIT)
            {
                running = 0;
            }
        } 

        emulate_instruction(&cpu);
        video_buffer_to_screen(&cpu);
        update_screen();

    }
}

void video_buffer_to_screen(Cpu8080 *cpu) 
{
    for (int i = VIDEO_RAM_START; i < (VIDEO_RAM_END-1); i++) {
        for (int bit = 0; bit < 8; bit++) {
            uint8_t bit_choosed = (cpu->memory[i] >> (7 - bit)) & 1;

            Uint8 r, g, b, a = 255;

            if (bit_choosed == 0) {
                r = 0;
                g = 0;
                b = 0;
            } else {
                r = 255;
                g = 255;
                b = 255;
            }

            unsigned index = (((i - VIDEO_RAM_START) * 8) + bit);

            screen_buffer[index] = SDL_MapRGBA(SDL_AllocFormat(SDL_PIXELFORMAT_RGBA8888), r, g, b, a);
        }
    }
}

int main()
{
	init_sdl();
	create_window();
	create_render();
	create_texture();
    init_cpu(cpu);
	load_rom();
    load_rom_to_memory(&cpu);
    init_sdl_screen_buffer();

    video_buffer_to_screen(&cpu);
    update_screen();
	intel8080_main();

	finish_and_free();
    return 0;
}
