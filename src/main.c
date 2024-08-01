#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>

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

#define set_bit(value, bit) value |= bit
#define unset_bit(value, bit) value &= bit 

void init_cpu(Cpu8080 *cpu) 
{
    cpu->registers = (Registers){0};
    cpu->memory = (uint8_t*)calloc(0x10000, sizeof(uint8_t));
    if (!cpu->memory) {
        perror("Memory allocation error");
        exit(EXIT_FAILURE);
    }
    printf("cpu initialized\n");
}

void set_flag(Cpu8080 *cpu, uint16_t result16, uint8_t value, uint8_t carry)
{
    uint8_t result8 = result16 & 0xFF;

    if (result16 == 0)
    {
        cpu->registers.F |= FLAG_ZERO; 
    }
    else {
        cpu->registers.F &= ~FLAG_ZERO;
    }

    if (result8 & 0x80)
    {
        cpu->registers.F |= FLAG_SIGN; 
    }
    else {
        cpu->registers.F &= ~FLAG_SIGN;
    }

    bool parity = __builtin_parity(result8);
   
    if (!parity) {
        cpu->registers.F |= FLAG_PARITY;
    } else {
        cpu->registers.F &= ~FLAG_PARITY;
    }

    if (result16 & 0xFF00) {
        cpu->registers.F |= FLAG_CARRY;
    } else {
        cpu->registers.F &= ~FLAG_CARRY;
    }

    if ((cpu->registers.A & 0x0F) < ((value + carry) & 0x0F)) {
        cpu->registers.F |= FLAG_AUX_CARRY;
    } else {
        cpu->registers.F &= ~FLAG_AUX_CARRY;
    }
}

uint16_t twoU8_to_u16adress(uint8_t byte1, uint8_t byte2)
{
    return (byte2 << 8 | byte1);
}

uint16_t twoU8_to_u16value(uint8_t byte1, uint8_t byte2)
{
    return (byte1 << 8 | byte2);
}

void NOP() 
{
    return;
}

// Load register pair immediate
void LXI(Cpu8080 *cpu, uint8_t *reg_low, uint8_t *reg_high) 
{
    *reg_low = cpu->rom[cpu->registers.pc + 1];
    *reg_high = cpu->rom[cpu->registers.pc + 2];
    cpu->registers.pc += 2;
}

void LDA(Cpu8080 *cpu, uint16_t adress)
{
    cpu->registers.A = cpu->memory[adress];
}

void LDAX(Cpu8080 *cpu, uint8_t *_register1, uint8_t *_register2)
{
    cpu->registers.A = twoU8_to_u16value(*_register1, *_register2);
}

void STAX(Cpu8080 *cpu, uint8_t *_register1, uint8_t *_register2)
{
    uint16_t adress = twoU8_to_u16adress(*_register1, *_register2);
    cpu->memory[adress] = cpu->registers.A;
}

void STA(Cpu8080 *cpu, uint8_t value)
{
    cpu->registers.A = value;
    cpu->registers.pc++;
}

void STC(Cpu8080 *cpu)
{
    cpu->registers.F |= FLAG_CARRY;
}

void MOV_reg_to_reg(Cpu8080 *cpu, uint8_t *target, uint8_t *source)
{
    *target = *source;
}

void MOV_mem_to_reg(Cpu8080 *cpu, uint8_t *target)
{
    uint16_t adress = twoU8_to_u16adress(cpu->registers.H, cpu->registers.L);

    *target = cpu->memory[adress];
}

void MOV_reg_to_mem(Cpu8080 *cpu, uint8_t *source)
{
    uint16_t adress = twoU8_to_u16adress(cpu->registers.H, cpu->registers.L);

    cpu->memory[adress] = *source;
}

void MOV_im_to_reg(Cpu8080 *cpu, uint8_t *target, uint8_t value)
{
    *target = value;
}

void ADD(Cpu8080 *cpu, uint8_t *_register)
{
    cpu->registers.A += *_register;
}

void ADC(Cpu8080 *cpu, uint8_t *_register)
{
    uint16_t result16 = *_register + cpu->registers.A + (cpu->registers.F & 1);
    uint8_t result8 = result16 & 0xFF;

    set_flag(cpu, result16, *_register, cpu->registers.F & 1);

    cpu->registers.A = result8;
}

void ACI(Cpu8080 *cpu, uint8_t value)
{
    uint16_t temp = value + cpu->registers.A + (cpu->registers.F & 1);

    set_flag(cpu, temp, value, cpu->registers.F & 1);

    cpu->registers.A = temp & 0xFF;
}

void SUI(Cpu8080 *cpu, uint8_t value)
{
    uint16_t result16 = cpu->registers.A - value;
    uint8_t result8 = result16 & 0xFF;

    set_flag(cpu, result16, value, 0);

    cpu->registers.A = result8;
}

void SUB(Cpu8080 *cpu, uint8_t *_register)
{
    uint16_t result16 = cpu->registers.A - *_register;
    uint8_t result8 = result16 & 0xFF;

    set_flag(cpu, result16, *_register, 0);

    cpu->registers.A = result8;
}

void SBB(Cpu8080 *cpu, uint8_t *_register)
{
    uint16_t result16 = cpu->registers.A - (*_register - (cpu->registers.F & 1));
    uint8_t result8 = result16 & 0xFF;

    set_flag(cpu, result16, *_register, cpu->registers.F & 1);

    cpu->registers.A = result8;
}

void ANA(Cpu8080 *cpu, uint8_t *_register)
{
    cpu->registers.A = cpu->registers.A & *_register;
    set_flag(cpu, cpu->registers.A, 0, 0);
}

void XRA(Cpu8080 *cpu, uint8_t *_register) 
{
    cpu->registers.A ^= *_register;
    set_flag(cpu, cpu->registers.A, 0, 0);
}


void ORA(Cpu8080 *cpu, uint8_t *_register)
{
    cpu->registers.A = cpu->registers.A | *_register;
    set_flag(cpu, cpu->registers.A, 0, 0);
}

void XRI(Cpu8080 *cpu, uint8_t value)
{
    cpu->registers.A = cpu->registers.A ^ value;
    set_flag(cpu, cpu->registers.A, 0, 0);
}

void CMP(Cpu8080 *cpu, uint8_t *_register) 
{
    uint16_t result16 = cpu->registers.A - *_register;
    set_flag(cpu, result16, *_register, 0);
}

void DCX(Cpu8080 *cpu, uint8_t *_register1, uint8_t *_register2) 
{
    uint16_t byte_combined = twoU8_to_u16value(*_register1, *_register2);
    byte_combined -= 1;

    *_register1 = (uint8_t)(byte_combined >> 8);
    *_register2 = (uint8_t)(byte_combined & 0xFF);
}

void DCX_16(Cpu8080 *cpu, uint16_t *_register)
{
    (*_register)--;
}

void DCR(Cpu8080 *cpu, uint8_t *_register)
{
    (*_register)-=1;
    set_flag(cpu, *_register, 0, 0);
}

void INX(Cpu8080 *cpu, uint8_t *_register1, uint8_t *_register2) 
{
    uint16_t byte_combined = twoU8_to_u16value(*_register1, *_register2);
    byte_combined -= 1;

    *_register1 = (uint8_t)(byte_combined >> 8);
    *_register2 = (uint8_t)(byte_combined & 0xFF);
}

void INX_16(Cpu8080 *cpu, uint16_t *_register)
{
    (*_register)++;
}

void INR(Cpu8080 *cpu, uint8_t *_register)
{
    (*_register)++;
    set_flag(cpu, *_register, 0, 0);
}

void INR_16(Cpu8080 *cpu, uint16_t *_register)
{
    (*_register)++;
}


void LHLD(Cpu8080 *cpu)
{
    unsigned int *pc = &cpu->registers.pc;
    uint16_t adress = twoU8_to_u16adress(cpu->rom[*pc+1], cpu->rom[*pc+2]);

    cpu->registers.L = cpu->memory[adress];
    cpu->registers.H = cpu->memory[adress + 1];

    pc+=2;
}

void CPI(Cpu8080 *cpu, uint8_t value)
{
    uint16_t result16 = cpu->registers.A - value;
    set_flag(cpu, result16, 0, 0);
    cpu->registers.pc++;
}

void CMA(Cpu8080 *cpu)
{
    cpu->registers.A = ~cpu->registers.A;
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
}


void DAD_16(Cpu8080 *cpu, uint16_t *_register) 
{
    uint8_t H = cpu->registers.H;
    uint8_t L = cpu->registers.L;

    uint16_t HL = twoU8_to_u16value(H, L);

    HL += *_register;

    cpu->registers.H = (uint8_t)(HL >> 8);
    cpu->registers.L = (uint8_t)(HL & 0xFF);
}

void ADI(Cpu8080 *cpu)
{
	uint8_t *A   = &cpu->registers.A;
    uint8_t *ROM = &cpu->rom;
    uint8_t *PC  = &cpu->registers.pc;

    uint8_t value = ROM[(*PC) + 1];
    
	*A += value;
	set_flag(cpu, *A, 0, 0);

	*PC++;

}

void RLC(Cpu8080 *cpu)
{
   uint8_t *A = &cpu->registers.A;
   uint8_t *F = &cpu->registers.F;

   // get 7th bit
   uint8_t prev_bit_7 = *A & ~BIT_7;
   uint8_t prev_bit_0 = *A & ~BIT_0;
   
   *A = *A << 1;

   // bit[0] = prev_bit[7]
   *A |= (prev_bit_7 << 7);

   // set CY bit with the bit[0]
   *F |= prev_bit_0;
   
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
    
}

void RAL(Cpu8080 *cpu)
{
    uint8_t prev_carry = cpu->registers.F & FLAG_CARRY;
    uint8_t prev_bit_7 = cpu->registers.A & BIT_7;

    cpu->registers.A = cpu->registers.A << 1;
    
    cpu->registers.A &= ~prev_carry; 

    cpu->registers.F &= ~prev_bit_7;

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
    
}

void SHLD(Cpu8080 *cpu)
{
    uint16_t adress = twoU8_to_u16adress(cpu->registers.H, cpu->registers.L);

    cpu->memory[adress] = cpu->registers.L;
    cpu->memory[adress+1] = cpu->registers.H;
}

void CMC(Cpu8080 *cpu)
{
    uint8_t *F = &cpu->registers.F;

    //get CY bit
    uint8_t CY_BIT =  *F & 0xFF;
    uint8_t NOT_CY_BIT = ~CY_BIT & 0xFF;
}

void POP(Cpu8080 *cpu, uint8_t *register_1, uint8_t *register_2)
{
    uint16_t *sp = &cpu->registers.sp;
    unsigned char *memory = (unsigned char *)&cpu->memory;
    int index = (int)sp;
    *register_2 = memory[index];
    *register_1 = memory[index+1];

    *sp+=2;
}

void PUSH(Cpu8080 *cpu, uint8_t *register_1, uint8_t *register_2)
{   	
    uint16_t *sp = &cpu->registers.sp;
    unsigned char *memory = (unsigned char *)&cpu->memory;
    int index = (int)sp;
    memory[index]   = *register_2;
    memory[index-1] = *register_1;

    *sp-=2;
}

void JNC(Cpu8080 *cpu)
{
    
    uint8_t *F = &cpu->registers.F;
    uint8_t *ROM = &cpu->rom;
    uint8_t *PC = &cpu->registers.pc;

    uint8_t adress_to_pc = ROM[(*PC) + 1];

    uint8_t CY = *F & FLAG_CARRY;


    if (CY != 0)
    {
	*PC = adress_to_pc;
    }

    (*PC)++;
}

void JP(Cpu8080 *cpu)
{
    uint8_t *ROM = &cpu->rom;
    uint8_t *PC  = &cpu->registers.pc;
    uint8_t adress_low = ROM[*PC];
    uint8_t adress_high = ROM[(*PC) + 1];
    uint16_t adress_pc = twoU8_to_u16adress(adress_low, adress_high);
    
    uint8_t *F = &cpu->registers.F;

    // if Parity bit is true, then
    if (*F & FLAG_PARITY)
    {
	*PC = adress_pc;
    }

    *PC += 2;
}

void JPO (Cpu8080 *cpu)
{
    uint8_t *ROM = &cpu->rom;
    uint8_t *PC  = &cpu->registers.pc;
    uint8_t adress_low = ROM[*PC];
    uint8_t adress_high = ROM[(*PC) + 1];
    uint16_t adress_pc = twoU8_to_u16adress(adress_low, adress_high);
    
    uint8_t *F = &cpu->registers.F;

    // if Sign bit is true, then
    if (*F & FLAG_SIGN)
    {
	*PC = adress_pc;
    }

    *PC += 2;    
}

void JM (Cpu8080 *cpu)
{
    uint8_t *ROM = &cpu->rom;
    uint8_t *PC  = &cpu->registers.pc;
    uint8_t adress_low = ROM[*PC];
    uint8_t adress_high = ROM[(*PC) + 1];
    uint16_t adress_pc = twoU8_to_u16adress(adress_low, adress_high);
    
    uint8_t *F = &cpu->registers.F;

    // if Sign bit is false, then
    if (! *F & FLAG_SIGN)
    {
	*PC = adress_pc;
    }

    *PC += 2;    
}

void JNZ (Cpu8080 *cpu)
{

    uint8_t *ROM = &cpu->rom;
    uint8_t *PC  = &cpu->registers.pc;
    uint8_t adress_low = ROM[*PC];
    uint8_t adress_high = ROM[(*PC) + 1];
    uint16_t adress_pc = twoU8_to_u16adress(adress_low, adress_high);
    
    uint8_t *F = &cpu->registers.F;

    // if ZERO bit is false, then
    if (! *F & FLAG_ZERO)
    {
		*PC = adress_pc;
    }

    *PC += 2;      
}

void JMP(Cpu8080 *cpu)
{
    uint8_t *ROM = &cpu->rom;
    uint8_t *PC  = &cpu->registers.pc;
    uint8_t adress_low = ROM[*PC];
    uint8_t adress_high = ROM[(*PC) + 1];
    uint16_t adress_pc = twoU8_to_u16adress(adress_low, adress_high);
    
    *PC = adress_pc;
}

void CP(Cpu8080 *cpu)
{
    uint8_t *ROM = &cpu->rom;
    uint8_t *PC  = &cpu->registers.pc;
    uint8_t adress_low = ROM[*PC];
    uint8_t adress_high = ROM[(*PC) + 1];
    uint16_t adress_pc = twoU8_to_u16adress(adress_low, adress_high);
    
    uint8_t *F = &cpu->registers.F;

    // if Parity bit is true, then
    if (*F & FLAG_PARITY)
    {
	*PC = adress_pc;
    }

    *PC += 2; 
}

void XCHG(Cpu8080 *cpu)
{
    uint8_t prev_H = cpu->registers.H;
    uint8_t prev_L = cpu->registers.L;

    cpu->registers.H = cpu->registers.D;
    cpu->registers.L = cpu->registers.E;

    cpu->registers.D = prev_H;
    cpu->registers.E = prev_L; 
}

void SPHL(Cpu8080 *cpu)
{
	uint16_t HL = twoU8_to_u16value(&cpu->registers.H, &cpu->registers.L);
	uint16_t *SP = &cpu->registers.sp;

	*SP = HL;
}

void ORI(Cpu8080 *cpu)
{
	uint8_t *A = &cpu->registers.A;
	uint8_t *ROM = &cpu->rom;
    uint8_t *PC  = &cpu->registers.pc;
    uint8_t data = ROM[(*PC) + 1];
	
	*A |= data;

	set_flag(cpu, *A, 0, 0);
	(*PC)++;
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
	unsigned int *PC = &cpu->registers.pc;	 
	unsigned int adress = (unsigned int)twoU8_to_u16value(cpu->rom[*PC+1], cpu->rom[*PC+2]);
	
	CALL(cpu, adress);	
}

void CM (Cpu8080 *cpu)
{
    uint8_t *ROM = &cpu->rom;
    uint8_t *PC  = &cpu->registers.pc;
    uint8_t adress_low = ROM[*PC];
    uint8_t adress_high = ROM[(*PC) + 1];
    unsigned int adress_pc = (unsigned int)twoU8_to_u16adress(adress_low, adress_high);
    
    uint8_t *F = &cpu->registers.F;

    // if Sign bit is false, then
    if (! *F & FLAG_SIGN)
    {
		CALL(cpu, adress_pc);
    }

    *PC += 2;    
}

void CZ (Cpu8080 *cpu)
{
    uint8_t *ROM = &cpu->rom;
    uint8_t *PC  = &cpu->registers.pc;
    uint8_t adress_low = ROM[*PC];
    uint8_t adress_high = ROM[(*PC) + 1];
    unsigned int adress_pc = (unsigned int)twoU8_to_u16adress(adress_low, adress_high);
    
    uint8_t *F = &cpu->registers.F;

    // if Zero bit is true, then
    if (*F & FLAG_ZERO)
    {
		CALL(cpu, adress_pc);
    }

    *PC += 2;    
}

void CNZ (Cpu8080 *cpu)
{
    uint8_t *ROM = &cpu->rom;
    uint8_t *PC  = &cpu->registers.pc;
    uint8_t adress_low = ROM[*PC];
    uint8_t adress_high = ROM[(*PC) + 1];
    unsigned int adress_pc = (unsigned int)twoU8_to_u16adress(adress_low, adress_high);
    
    uint8_t *F = &cpu->registers.F;

    // if Zero bit is false, then
    if (! *F & FLAG_ZERO)
    {
		CALL(cpu, adress_pc);
    }

    *PC += 2;    
}

void CC (Cpu8080 *cpu)
{
    uint8_t *ROM = &cpu->rom;
    uint8_t *PC  = &cpu->registers.pc;
    uint8_t adress_low = ROM[*PC];
    uint8_t adress_high = ROM[(*PC) + 1];
    unsigned int adress_pc = (unsigned int)twoU8_to_u16adress(adress_low, adress_high);
    
    uint8_t *F = &cpu->registers.F;

    // if Carry bit is true, then
    if (*F & FLAG_CARRY)
    {
		CALL(cpu, adress_pc);
    }

    *PC += 2;    
}

void CNC (Cpu8080 *cpu)
{
    uint8_t *ROM = &cpu->rom;
    uint8_t *PC  = &cpu->registers.pc;
    uint8_t adress_low = ROM[*PC];
    uint8_t adress_high = ROM[(*PC) + 1];
    unsigned int adress_pc = (unsigned int)twoU8_to_u16adress(adress_low, adress_high);
    
    uint8_t *F = &cpu->registers.F;

    // if Carry bit is false, then
    if (! *F & FLAG_CARRY)
    {
		CALL(cpu, adress_pc);
    }

    *PC += 2;    
}

void RET(Cpu8080 *cpu)
{
	unsigned int *PC = &cpu->registers.pc;
	uint16_t     *SP = &cpu->registers.sp;
	uint8_t		 *memory = (uint8_t*)&cpu->memory;
	
	unsigned int Higher = memory[*SP+1];
	unsigned int Lower = memory[*SP];	

	SP+=2;
	
	*PC = twoU8_to_u16value(Higher, Lower); 
}

void emulate(Cpu8080 *cpu) 
{
    uint8_t* A = &cpu->registers.A;
    uint8_t* B = &cpu->registers.B;
    uint8_t* C = &cpu->registers.C;
    uint8_t* D = &cpu->registers.D;
    uint8_t* E = &cpu->registers.E;
    uint8_t* H = &cpu->registers.H;
    uint8_t* L = &cpu->registers.L;

    uint16_t* sp  = &cpu->registers.sp;
    unsigned int* pc  = &cpu->registers.pc;
    uint8_t* rom = (uint8_t*)&cpu->rom;

    cpu->rom = get_rom();
    unsigned int rom_size = get_rom_size();

    if (cpu->rom == NULL) {
        fprintf(stderr, "Failed to load ROM\n");
        return;
    }

    init_cpu(cpu);

    log_8080("emulation started\n");

    start_clock_debug(cpu);

    while (cpu->registers.pc < rom_size) 
    {
        getchar();
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
                INX(cpu, &cpu->registers.B, &cpu->registers.C);
                break;

            case 0x04:
                INR(cpu, &cpu->registers.B);
                break;

            case 0x05:
                DCR(cpu, &cpu->registers.B);
                break;

            case 0x06:
                MOV_im_to_reg(cpu, &cpu->registers.B, cpu->rom[cpu->registers.pc+1]);
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
                DCX(cpu, &cpu->registers.B, &cpu->registers.C);
                break;

            case 0x0C:
                INR(cpu, &cpu->registers.C);
                break;

			case 0x0D:
				DCR(cpu, &cpu->registers.C);
				break;

            case 0x0E:
                MOV_im_to_reg(cpu, &cpu->registers.C, cpu->rom[cpu->registers.pc+1]);
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
                INX(cpu, &cpu->registers.D, &cpu->registers.E);
                break;

            case 0x14:
                INR(cpu, &cpu->registers.D);
                break;

            case 0x15:
                DCR(cpu, &cpu->registers.D);
                break;

            case 0x16:
                MOV_im_to_reg(cpu, &cpu->registers.D, cpu->rom[cpu->registers.pc+1]);
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
                DCX(cpu, &cpu->registers.D, &cpu->registers.E);
                break;

            case 0x1C:
                INR(cpu, &cpu->registers.E);
                break;

            case 0x1D:
                DCR(cpu, &cpu->registers.E);
                break;

            case 0x1E:
                MOV_im_to_reg(cpu, &cpu->registers.E, cpu->rom[cpu->registers.pc+1]);
                break;

			case 0x1F:
				RAR(cpu);
				break;

			case 0x21:
                {

                    cpu->rom[cpu->registers.pc+1];
                    LXI(cpu, &cpu->registers.H, &cpu->registers.L);
                }
                break;

			case 0x22:
				SHLD(cpu);
				break;
            
			case 0x23:
				INX(cpu, &cpu->registers.H, &cpu->registers.L);
                break;

            case 0x24:
                INR(cpu, &cpu->registers.H);
                break;

            case 0x25:
                DCR(cpu, &cpu->registers.H);
                break;

            case 0x26:
                MOV_im_to_reg(cpu, &cpu->registers.H, cpu->rom[cpu->registers.pc+1]);
                break;

            case 0x29:
                DAD(cpu, &cpu->registers.H, &cpu->registers.L);
                break;

            case 0x2a:
                LHLD(cpu);
                break;

            case 0x2b:
                DCX(cpu, &cpu->registers.H, &cpu->registers.L);
                break;

            case 0x2c:
                INR(cpu, &cpu->registers.L);
                break;

            case 0x2d:
                DCR(cpu, &cpu->registers.L);
                break;

            case 0x2e:
                MOV_im_to_reg(cpu, &cpu->registers.L, cpu->rom[cpu->registers.pc+1]);
                break;

            case 0x2f:
                CMA(cpu);
                break;


            case 0x31:
                {
                    uint8_t high = (uint8_t)(cpu->registers.sp >> 8);
                    uint8_t low = (uint8_t)(cpu->registers.sp & 0xFF);
                    LXI(cpu, &high, &low);
                }
                break;

            case 0x32:
                {
                    STA(cpu, cpu->rom[cpu->registers.pc+1]);
                }
                break;

            case 0x33:
                INX_16(cpu, &cpu->registers.sp);
                break;

            case 0x34:
                {
                    uint8_t adress = twoU8_to_u16value(cpu->registers.H, cpu->registers.L);
                    INR(cpu, &cpu->memory[adress]);
                }
                break;

            case 0x35:
                {
                    uint8_t adress = twoU8_to_u16value(cpu->registers.H, cpu->registers.L);
                    DCR(cpu, &cpu->registers.A);
                }
                break;

            case 0x36:
                {
                    uint8_t adress = twoU8_to_u16value(cpu->registers.H, cpu->registers.L);
                    MOV_reg_to_mem(cpu, &cpu->memory[adress]);
                }
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
                }
                break;

            case 0x3B:
                DCX_16(cpu, &cpu->registers.sp);
                break;

            case 0x3C:
                INR(cpu, &cpu->registers.A);
                break;

            case 0x3D:
                DCR(cpu, &cpu->registers.A);
                break;

            case 0x3E:
                MOV_im_to_reg(cpu, &cpu->registers.A, cpu->rom[cpu->registers.pc+1]);
                break;

			case 0x3F:
				CMC(cpu);
				break;

            // MOVs
            case 0x40:
                MOV_reg_to_reg(cpu, B, B);
                break;

            case 0x41:
                MOV_reg_to_reg(cpu, B, C);
                break;

            case 0x42:
                MOV_reg_to_reg(cpu, B, D);
                break;

            case 0x43:
                MOV_reg_to_reg(cpu, B, E);
                break;

            case 0x44:
                MOV_reg_to_reg(cpu, B, H);
                break;

            case 0x45:
                MOV_reg_to_reg(cpu, B, L);
                break;

            case 0x46:
                MOV_mem_to_reg(cpu, B);
                break;

            case 0x47:
                MOV_reg_to_reg(cpu, B, A);
                break;

            case 0x48:
                MOV_reg_to_reg(cpu, C, B);
                break;

            case 0x49:
                MOV_reg_to_reg(cpu, C, C);
                break;

            case 0x4A:
                MOV_reg_to_reg(cpu, C, D);
                break;

            case 0x4B:
                MOV_reg_to_reg(cpu, C, E);
                break;

            case 0x4C:
                MOV_reg_to_reg(cpu, C, H);
                break;

            case 0x4D:
                MOV_reg_to_reg(cpu, C, L);
                break;

            case 0x4E:
                MOV_mem_to_reg(cpu, C);
                break;

            case 0x4F:
                MOV_reg_to_reg(cpu, C, A);
                break;

            case 0x50:
                MOV_reg_to_reg(cpu, D, B);
                break;

            case 0x51:
                MOV_reg_to_reg(cpu, D, C);
                break;

            case 0x52:
                MOV_reg_to_reg(cpu, D, D);
                break;

            case 0x53:
                MOV_reg_to_reg(cpu, D, E);
                break;

            case 0x54:
                MOV_reg_to_reg(cpu, D, H);
                break;

            case 0x55:
                MOV_reg_to_reg(cpu, D, L);
                break;

            case 0x56:
                MOV_mem_to_reg(cpu, D);
                break;

            case 0x57:
                MOV_reg_to_reg(cpu, D, A);
                break;

            case 0x58:
                MOV_reg_to_reg(cpu, E, B);
                break;

            case 0x59:
                MOV_reg_to_reg(cpu, E, C);
                break;

            case 0x5A:
                MOV_reg_to_reg(cpu, E, D);
                break;

            case 0x5B:
                MOV_reg_to_reg(cpu, E, E);
                break;

            case 0x5C:
                MOV_reg_to_reg(cpu, E, H);
                break;

            case 0x5D:
                MOV_reg_to_reg(cpu, E, L);
                break;

            case 0x5E:
                MOV_mem_to_reg(cpu, E);
                break;

            case 0x5F:
                MOV_reg_to_reg(cpu, E, A);
                break;

            case 0x60:
                MOV_reg_to_reg(cpu, H, B);
                break;

            case 0x61:
                MOV_reg_to_reg(cpu, H, C);
                break;

            case 0x62:
                MOV_reg_to_reg(cpu, H, D);
                break;

            case 0x63:
                MOV_reg_to_reg(cpu, H, E);
                break;

            case 0x64:
                MOV_reg_to_reg(cpu, H, H);
                break;

            case 0x65:
                MOV_reg_to_reg(cpu, H, L);
                break;

            case 0x66:
                MOV_mem_to_reg(cpu, H);
                break;

            case 0x67:
                MOV_reg_to_reg(cpu, H, A);
                break;

            case 0x68:
                MOV_reg_to_reg(cpu, L, B);
                break;

            case 0x69:
                MOV_reg_to_reg(cpu, L, C);
                break;

            case 0x6A:
                MOV_reg_to_reg(cpu, L, D);
                break;

            case 0x6B:
                MOV_reg_to_reg(cpu, L, E);
                break;

            case 0x6C:
                MOV_reg_to_reg(cpu, L, H);
                break;

            case 0x6D:
                MOV_reg_to_reg(cpu, L, L);
                break;

            case 0x6E:
                MOV_mem_to_reg(cpu, L);
                break;

            case 0x6F:
                MOV_reg_to_reg(cpu, L, A);
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
                // HALT
                break;

            case 0x77:
                MOV_reg_to_mem(cpu, A);
                break;

            case 0x78:
                MOV_reg_to_reg(cpu, A, B);
                break;

            case 0x79:
                MOV_reg_to_reg(cpu, A, C);
                break;

            case 0x7a:
                MOV_reg_to_reg(cpu, A, D);
                break;

            case 0x7b:
                MOV_reg_to_reg(cpu, A, E);
                break;

            case 0x7c:
                MOV_reg_to_reg(cpu, A, H);
                break;

            case 0x7d:
                MOV_reg_to_reg(cpu, A, L);
                break;

            case 0x7e:
                MOV_mem_to_reg(cpu, A);
                break;

            case 0x7f:
                MOV_reg_to_reg(cpu, A, A);
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
                }
                break;

		case 0xBF:
            CMP(cpu, &cpu->registers.A);
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
		
		case 0xC9:
			RET(cpu);
			break;

		case 0xCC:
			CZ(cpu);
			break;
		
		case 0xCD: 
			CALL_adr(cpu);
			break;

		case 0xCF:
			RST(cpu, 0x08);
			break;

	    case 0xD1:
			POP(cpu, &cpu->registers.D, &cpu->registers.E);
			break;

	    case 0xD2:
			JNC(cpu);
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
		
		case 0xDC:
			CC(cpu);
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
	
	    case 0xE5:
			PUSH(cpu, &cpu->registers.H, &cpu->registers.L);
			break;

		case 0xE7:
			RST(cpu, 0x20);
			break;
	
	    case 0xEB:	
			XCHG(cpu);
			break;
		
		case 0xEE:
            {
                uint16_t mem_adress = twoU8_to_u16adress(cpu->registers.H, cpu->registers.L); 
                uint8_t value = cpu->memory[mem_adress];

                XRI(cpu, value);
            }
            break;

		case 0xEF:
			RST(cpu, 0x28);
			break;

		case 0xF1:
			POP(cpu, &cpu->registers.A, &cpu->registers.F);
			break;
	
	    case 0xF2:
			JP(cpu);
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

		case 0xF9:
			SPHL(cpu);
			break;
		
		case 0xFA:
			JM(cpu);
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
            break;
        }

        cpu->registers.pc++;
        update_clock_debug(cpu);
    }

    free(cpu->rom);
    free(cpu->memory);
}

int main() 
{
    Cpu8080 cpu;
    emulate(&cpu);
    return 0;
}
