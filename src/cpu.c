#include <stdint.h>
#include <SDL2/SDL.h>
#include <errno.h>

#include "cpu.h"
#include "helper.h"
#include "debug.h"
#include "rom.h"
#include "main.h"

unsigned int rom_size;

// Global alias
uint8_t* A;
uint8_t* B;
uint8_t* C;
uint8_t* D;
uint8_t* E;
uint8_t* H;
uint8_t* L;


Cpu8080* init_cpu() 
{
    Cpu8080 *cpu = (Cpu8080*)malloc(sizeof(Cpu8080));;

    cpu->memory = (uint8_t*)calloc(TOTAL_MEMORY_SIZE, sizeof(uint8_t));

    if (! cpu->memory) {
        fprintf(stderr, "Error on memory allocation: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }

    A = &cpu->registers.A;
    B = &cpu->registers.B;
    C = &cpu->registers.C;
    D = &cpu->registers.D;
    E = &cpu->registers.E;
    H = &cpu->registers.H;
    L = &cpu->registers.L;

    cpu->registers.A = 0;
    cpu->registers.B = 0;
    cpu->registers.C = 0;
    cpu->registers.D = 0;
    cpu->registers.E = 0;
    cpu->registers.H = 0;
    cpu->registers.L = 0;

    cpu->registers.F.cy = 0;
    cpu->registers.F.p  = 0;
    cpu->registers.F.ac = 0;
    cpu->registers.F.z  = 0;
    cpu->registers.F.s  = 0;

    cpu->registers.sp = 32;
    
    return cpu;
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

int parity(int x, int size)
{
	int i;
	int p = 0;
	x = (x & ((1<<size)-1));

	for (i=0; i<size; i++)
	{
		if (x & 0x1) p++;
		x = x >> 1;
	}
	return (0 == (p & 0x1));
}

void LogicFlagsA(Cpu8080 *cpu)
{
	cpu->registers.F.cy = cpu->registers.F.ac = 0;
	cpu->registers.F.z = (cpu->registers.A == 0);
	cpu->registers.F.s = (0x80 == (cpu->registers.A & 0x80));
	cpu->registers.F.p = parity(cpu->registers.A, 8);
}

void ArithFlagsA(Cpu8080 *cpu, uint16_t res)
{
	cpu->registers.F.cy = (res > 0xff);
	cpu->registers.F.z = ((res&0xff) == 0);
	cpu->registers.F.s = (0x80 == (res & 0x80));
	cpu->registers.F.p = parity(res&0xff, 8);
}

void NOP(Cpu8080 *cpu) 
{
    cpu->registers.pc++;
    return;
}

// Load register pair immediate
void LXI(Cpu8080 *cpu, uint8_t *reg_high, uint8_t *reg_low) 
{
    *reg_low = cpu->rom[cpu->registers.pc + 1];
    *reg_high = cpu->rom[cpu->registers.pc + 2];
    cpu->registers.pc += 3;
}

void LDA(Cpu8080 *cpu)
{
    uint16_t address = (cpu->rom[cpu->registers.pc + 2] << 8) | cpu->rom[cpu->registers.pc + 1];
    cpu->registers.A = cpu->memory[address];

    cpu->registers.pc += 3;
}

void LDAX(Cpu8080 *cpu, uint8_t *lsb, uint8_t *msb)
{
    uint16_t address = twoU8_to_u16value(*lsb, *msb);

    cpu->registers.A = cpu->memory[address];

    cpu->registers.pc++;    
}

void STAX(Cpu8080 *cpu, uint8_t *_register1, uint8_t *_register2)
{
    uint16_t adress = twoU8_to_u16adress(*_register1, *_register2);
    cpu->memory[adress] = cpu->registers.A;
    cpu->registers.pc++;
}

void STA(Cpu8080 *cpu)
{
    uint16_t address = read_byte_address(cpu);

    cpu->memory[address] = cpu->registers.A;
    cpu->registers.pc += 3;
}

void STC(Cpu8080 *cpu)
{
    cpu->registers.F.cy = 1;
    cpu->registers.pc++;
}

void MOV_reg_to_reg(Cpu8080 *cpu, uint8_t *target, uint8_t *source)
{
    *target = *source;
    cpu->registers.pc++;
}

void MOV_mem_to_reg(Cpu8080 *cpu, uint8_t *target)
{
    uint16_t adress = twoU8_to_u16adress(cpu->registers.H, cpu->registers.L);

    *target = cpu->memory[adress];

    cpu->registers.pc += 1;
}

void MOV_reg_to_mem(Cpu8080 *cpu, uint8_t *source)
{
    uint16_t adress = twoU8_to_u16adress(cpu->registers.H, cpu->registers.L);

    cpu->memory[adress] = *source;

    cpu->registers.pc += 1;
}

void MOV_im_to_reg(Cpu8080 *cpu, uint8_t *target)
{
    uint8_t value = cpu->rom[cpu->registers.pc + 1];
    *target = value;
    cpu->registers.pc += 2;
}

void MOV_im_to_mem(Cpu8080 *cpu)
{
    uint16_t address = twoU8_to_u16adress(cpu->registers.H, cpu->registers.L);
    uint8_t value = cpu->memory[cpu->registers.pc + 1];
    cpu->memory[address] = value;
    cpu->registers.pc += 2;
}

void ADD(Cpu8080 *cpu, uint8_t byte)
{
    uint16_t result16 = byte + cpu->registers.A;
    uint8_t result8 = result16 & 0xFF;

    cpu->registers.A = result8;

    ArithFlagsA(cpu, result16);

    cpu->registers.pc++;
}

void ADC(Cpu8080 *cpu, uint8_t *_register)
{
    uint16_t result16 = *_register + cpu->registers.A + (cpu->registers.F.cy);

    ArithFlagsA(cpu, result16);

    cpu->registers.A = result16 & 0xFF;

    cpu->registers.pc++;
}

void ACI(Cpu8080 *cpu)
{
    uint8_t value = read_byte(cpu);
    uint16_t result16 = value + cpu->registers.A + (cpu->registers.F.cy);

    ArithFlagsA(cpu, result16);

    cpu->registers.A = result16 & 0xFF;

    cpu->registers.pc += 2;
}

void SBI(Cpu8080 *cpu)
{
    uint8_t value = read_byte(cpu);
    uint16_t result16 = cpu->registers.A - value - (cpu->registers.F.cy);

    cpu->registers.A = result16 & 0xFF;

    ArithFlagsA(cpu, result16);
    if (cpu->registers.A < result16)
        cpu->registers.F.cy = 1;

    cpu->registers.pc +=2;
}

void SUI(Cpu8080 *cpu)
{
    uint8_t value = read_byte(cpu);
    uint16_t result16 = cpu->registers.A - value;

    uint8_t old_A = cpu->registers.A;

    cpu->registers.A = result16 & 0xFF;

    ArithFlagsA(cpu, result16);
    cpu->registers.F.cy = 0;
    if (cpu->registers.A >= old_A)
        cpu->registers.F.cy = 1;

    cpu->registers.pc+=2;
}

void SUB(Cpu8080 *cpu, uint8_t *_register)
{
    uint16_t result16 = cpu->registers.A - *_register;

    cpu->registers.A = result16 & 0xFF;

    ArithFlagsA(cpu, result16);
    cpu->registers.F.cy = 0;
    if (cpu->registers.A >= result16)
        cpu->registers.F.cy = 1;

    cpu->registers.pc++;
}

void SBB(Cpu8080 *cpu, uint8_t *_register)
{
    uint16_t result16 = (cpu->registers.A - *_register) - (cpu->registers.F.cy);

    cpu->registers.A = result16 & 0xFF;

    ArithFlagsA(cpu, result16);
    if (cpu->registers.A < result16)
        cpu->registers.F.cy = 1;

    cpu->registers.pc++;
}

void ANA(Cpu8080 *cpu, uint8_t *_register)
{
    cpu->registers.A = cpu->registers.A & *_register;
    
    LogicFlagsA(cpu);

    cpu->registers.pc++;
}

void ANI(Cpu8080 *cpu)
{
	uint8_t value = read_byte(cpu);

    cpu->registers.A = cpu->registers.A & value;

    LogicFlagsA(cpu);

    cpu->registers.pc += 2;
}

void XRA(Cpu8080 *cpu, uint8_t *_register) 
{
    cpu->registers.A ^= *_register;
    LogicFlagsA(cpu);

    cpu->registers.pc++;
}

void ORA(Cpu8080 *cpu, uint8_t *_register)
{
    cpu->registers.A = cpu->registers.A | *_register;
    LogicFlagsA(cpu);

    cpu->registers.pc++;
}

void XRI(Cpu8080 *cpu)
{
    uint8_t value = read_byte(cpu);

    cpu->registers.A = cpu->registers.A ^ value;
    LogicFlagsA(cpu);

    cpu->registers.pc += 2;
}

void CMP(Cpu8080 *cpu, uint8_t *_register) 
{
    uint16_t result16 = cpu->registers.A - *_register;

    ArithFlagsA(cpu, result16);

    cpu->registers.pc++;
}

void DCX(Cpu8080 *cpu, uint8_t *_register1, uint8_t *_register2) 
{
    uint16_t byte_combined = twoU8_to_u16adress(*_register1, *_register2);
    byte_combined -= 1;

    *_register1 = (uint8_t)(byte_combined >> 8);
    *_register2 = (uint8_t)(byte_combined & 0xFF);

    cpu->registers.pc++;
}

void DCX_16(Cpu8080 *cpu, uint16_t *_register)
{
    (*_register)--;

    cpu->registers.pc++;
}

void DCR(Cpu8080 *cpu, uint8_t *_register)
{
    (*_register)-=1;
    ArithFlagsA(cpu, (*_register));

    cpu->registers.pc++;
}

void INX(Cpu8080 *cpu, uint8_t *_register1, uint8_t *_register2) 
{
    uint16_t byte_combined = twoU8_to_u16value(*_register1, *_register2);
    byte_combined += 1;

    *_register1 = (uint8_t)(byte_combined >> 8);
    *_register2 = (uint8_t)(byte_combined & 0xFF);

    cpu->registers.pc++;
}

void INX_16(Cpu8080 *cpu, uint16_t *_register)
{
    (*_register)++;

    cpu->registers.pc++;
}

void INR(Cpu8080 *cpu, uint8_t *_register)
{
    (*_register)++;
    ArithFlagsA(cpu, (*_register));

    cpu->registers.pc++;
}

void LHLD(Cpu8080 *cpu)
{
    uint16_t adress = read_byte_address(cpu);

    cpu->registers.L = cpu->memory[adress];
    cpu->registers.H = cpu->memory[adress + 1];

    cpu->registers.pc += 3;
}

void CPI(Cpu8080 *cpu)
{
    uint8_t value = read_byte(cpu);

    uint16_t result16 = cpu->registers.A - value;
    ArithFlagsA(cpu,result16);

    cpu->registers.pc += 2;
}

void CMA(Cpu8080 *cpu)
{
    cpu->registers.A = ~cpu->registers.A;
    LogicFlagsA(cpu);
    cpu->registers.pc += 1;
}

void DAD(Cpu8080 *cpu, uint8_t *_register1, uint8_t *_register2) 
{
    uint8_t H = cpu->registers.H;
    uint8_t L = cpu->registers.L;

    uint16_t byte_combined = twoU8_to_u16value(*_register1, *_register2);
    uint16_t HL = twoU8_to_u16adress(H, L);
    
    uint32_t result = (HL + byte_combined);

    cpu->registers.H = (uint8_t)(result >> 8);
    cpu->registers.L = (uint8_t)(result & 0xFF);

    if (result > 0xFFFF)
        cpu->registers.F.cy = 1;
    
    cpu->registers.pc += 1;
}

void DAD_16(Cpu8080 *cpu, uint16_t *_register) 
{
    uint8_t H = cpu->registers.H;
    uint8_t L = cpu->registers.L;

    uint16_t HL = twoU8_to_u16adress(H, L);

    HL += *_register;

    cpu->registers.H = (uint8_t)(HL >> 8);
    cpu->registers.L = (uint8_t)(HL & 0xFF);

    cpu->registers.pc += 2;
}

void ADI(Cpu8080 *cpu)
{
    uint8_t value = read_byte(cpu);
    
    uint16_t result16 = value + cpu->registers.A;

    cpu->registers.A = result16 & 0xFF;

	ArithFlagsA(cpu, result16);

	cpu->registers.pc +=2 ;
}

void RLC(Cpu8080 *cpu)
{
    uint8_t bit7 = cpu->registers.A & BIT_7;
    cpu->registers.A <<= 1;
    if (bit7) {
        cpu->registers.A |= BIT_0;
        cpu->registers.F.cy = 1;
    } else {
        cpu->registers.F.cy = 0;
    }

    cpu->registers.pc += 1;
}

void RRC(Cpu8080 *cpu)
{
    uint8_t *A = &cpu->registers.A;

    // get 7th bit
    uint8_t prev_bit_7 = *A & ~BIT_7;
    uint8_t prev_bit_0 = *A & ~BIT_0;

    *A = *A >> 1;

    // bit[7] = prev_bit[0]
    *A |= prev_bit_0 << 7;

    if (prev_bit_7)
        cpu->registers.F.cy = 1;
    else
        cpu->registers.F.cy = 0;

    cpu->registers.pc+=1;    
}

void RAL(Cpu8080 *cpu)
{
    uint8_t prev_carry = cpu->registers.F.cy;
    uint8_t prev_bit_7 = cpu->registers.A & BIT_7;

    cpu->registers.A <<= 1;
    
    if (prev_carry) {
        cpu->registers.A |= BIT_0;
    }

    if (prev_bit_7) {
        cpu->registers.F.cy = 1;
    } else {
        cpu->registers.F.cy = 0;
    }

    cpu->registers.pc+=1;
}

void RAR(Cpu8080 *cpu)
{
    uint8_t *A = &cpu->registers.A;

    uint8_t prev_bit_7 = *A & ~BIT_7;
    uint8_t prev_bit_0 = *A & ~BIT_0;

    *A = *A >> 1;

    if (prev_bit_0) {
        cpu->registers.F.cy = 1;
    } else {
        cpu->registers.F.cy = 0;
    }

    *A |= prev_bit_7 << 7;

    cpu->registers.pc+=1;
    
}

void SHLD(Cpu8080 *cpu)
{
    uint16_t adress = twoU8_to_u16adress(cpu->registers.H, cpu->registers.L);

    cpu->memory[adress]     = cpu->registers.L;
    cpu->memory[adress + 1] = cpu->registers.H;

    cpu->registers.pc += 3;
}

void CMC(Cpu8080 *cpu)
{
    cpu->registers.F.cy = ~cpu->registers.F.cy;

    cpu->registers.pc+=1;
}

void POP(Cpu8080 *cpu, uint8_t *register_1, uint8_t *register_2)
{
    uint16_t sp = cpu->registers.sp;

    *register_2 = cpu->memory[sp];
    *register_1 = cpu->memory[sp+=1];
    
    sp += 2;

    cpu->registers.sp = sp;
    cpu->registers.pc+=1;
}

void POP_PSW(Cpu8080 *cpu)
{
    uint16_t sp = cpu->registers.sp;

    cpu->registers.F = byteToFlags((uint8_t)cpu->memory[sp]);
    
    cpu->registers.A = cpu->memory[sp+1];
    
    sp += 2;

    cpu->registers.sp = sp;
    cpu->registers.pc+=1;
}

void PUSH(Cpu8080 *cpu, uint8_t *register_1, uint8_t *register_2)
{
    uint16_t sp = cpu->registers.sp;

    cpu->memory[sp-1] = *register_1;
    cpu->memory[sp-2] = *register_2;
    
    cpu->registers.sp -= 2;
    cpu->registers.pc +=1 ;
}

void PUSH_PSW(Cpu8080 *cpu)
{
    uint16_t sp = cpu->registers.sp;

    cpu->registers.F = byteToFlags((uint8_t)cpu->memory[sp]);
    
    cpu->registers.A = cpu->memory[sp+1];
    
    sp += 2;

    cpu->registers.sp = sp;
    cpu->registers.pc+=1;
}

void JC(Cpu8080 *cpu)
{
    unsigned int *PC = &cpu->registers.pc;

    uint16_t adress_to_pc = read_byte_address(cpu);

    if (cpu->registers.F.cy)
		*PC = adress_to_pc;
    else
        (*PC)+=3;
}

void JNC(Cpu8080 *cpu)
{
    unsigned int *PC = &cpu->registers.pc;

    uint16_t adress_to_pc = read_byte_address(cpu);

    if (! cpu->registers.F.cy)
		*PC = adress_to_pc;
    else
        (*PC) += 3;
}

void JP(Cpu8080 *cpu)
{
    unsigned int *PC = &cpu->registers.pc;

    uint16_t adress_to_pc = read_byte_address(cpu);

    // if Parity bit is TRUE, then
    if (cpu->registers.F.p)
	   *PC = adress_to_pc;
    else
        (*PC) += 3;
}

void JPO (Cpu8080 *cpu)
{
    unsigned int *PC = &cpu->registers.pc;

    uint16_t adress_to_pc = read_byte_address(cpu);

    // if Parity bit is FALSE, then
    if (! cpu->registers.F.p)
    	*PC = adress_to_pc;
    else
        (*PC) += 3;    
}

void JM (Cpu8080 *cpu)
{
    unsigned int *PC = &cpu->registers.pc;

    uint16_t adress_to_pc = read_byte_address(cpu);

    // if Sign bit is true, then
    if (cpu->registers.F.s)
	   *PC = adress_to_pc;
    else
        (*PC) += 3;    
}

void JNZ (Cpu8080 *cpu)
{
    unsigned int *PC = &cpu->registers.pc;

    uint16_t adress_to_pc = read_byte_address(cpu);

    // if ZERO bit is false, then
    if (! cpu->registers.F.z)
		*PC = adress_to_pc;
    else
        (*PC) += 3;      
}

void JZ (Cpu8080 *cpu)
{
    unsigned int *PC = &cpu->registers.pc;

    uint16_t adress_to_pc = read_byte_address(cpu);

    // if ZERO bit is true, then
    if (cpu->registers.F.z)
		*PC = adress_to_pc;
    else 
        (*PC) += 3;      
}

void JPE (Cpu8080 *cpu)
{
    JP(cpu);  
}

void JMP(Cpu8080 *cpu)
{
    unsigned int *PC  = &cpu->registers.pc;
    
    uint16_t adress = read_byte_address(cpu);

    *PC = adress;
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
	uint16_t HL = twoU8_to_u16adress(cpu->registers.H, cpu->registers.L);
	uint16_t *SP = &cpu->registers.sp;

	*SP = HL;

    cpu->registers.pc+=1;
}

void PCHL(Cpu8080 *cpu)
{
	unsigned int *PC = &cpu->registers.pc;
	uint16_t HL = twoU8_to_u16adress(cpu->registers.H, cpu->registers.L);
			 
	*PC = HL; 

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
    uint8_t data = read_byte(cpu);

    cpu->registers.A |= data;

	LogicFlagsA(cpu);
	
    cpu->registers.pc += 2;
}

void RST(Cpu8080* cpu, unsigned int new_pc_position)
{
	cpu->registers.pc = new_pc_position;
}

void CALL(Cpu8080 *cpu, unsigned int adress)
{
	unsigned int *PC = &cpu->registers.pc;
	uint16_t     *SP = &cpu->registers.sp;
	
    uint8_t Lower   = (*PC+3) >> 8;
    uint8_t Higher  = (*PC+3) & 0xff;
	
	cpu->memory[*SP-1] = Higher;
	cpu->memory[*SP-2] = Lower;

    /**
     * SP   ->  
     * SP-1 ->  Higher
     * SP-2 ->  Lower
     */

	*SP -= 2;
	*PC  = adress;

    /**
     * SP+1 ->  Higher
     * SP   ->  Lower
     */
    
}

void CALL_adr(Cpu8080 *cpu)
{
	unsigned int adress = read_byte_address(cpu);
	CALL(cpu, adress);	
}

void CM (Cpu8080 *cpu)
{
    unsigned int adress_pc = read_byte_address(cpu);

    // if Sign bit is false, then
    if (cpu->registers.F.s)
		CALL(cpu, adress_pc);
    else
        cpu->registers.pc += 3;
}

void CZ (Cpu8080 *cpu)
{
    unsigned int adress_pc = read_byte_address(cpu);
    
    // if Zero bit is true, then
    if (cpu->registers.F.z)
		CALL(cpu, adress_pc);
    else
        cpu->registers.pc += 3;    
}

void CNZ (Cpu8080 *cpu)
{
    unsigned int adress_pc = (unsigned int)read_byte_address(cpu);
    
    // if Zero bit is false, then
    if (! cpu->registers.F.z)
		CALL(cpu, adress_pc);
    else
        cpu->registers.pc += 3;  
}

void CC (Cpu8080 *cpu)
{
    unsigned int adress_pc = (unsigned int)read_byte_address(cpu);
    

    // if Carry bit is true, then
    if (cpu->registers.F.cy)
		CALL(cpu, adress_pc);
    else
        cpu->registers.pc += 3;  
}

void CNC (Cpu8080 *cpu)
{
    unsigned int adress_pc = (unsigned int)read_byte_address(cpu);

    // if Carry bit is false, then
    if (! (cpu->registers.F.cy))
		CALL(cpu, adress_pc);
    else
        cpu->registers.pc += 3;   
}

void CP(Cpu8080 *cpu)
{
    uint16_t adress_to_pc = read_byte_address(cpu);

    // if Parity bit is true, then
    if (cpu->registers.F.p)
	   CALL(cpu, adress_to_pc);
    else
        cpu->registers.pc += 3;
}

void CPO(Cpu8080 *cpu)
{
    uint16_t adress_to_pc = read_byte_address(cpu);

    // if Parity bit is false, then
    if (! cpu->registers.F.p)
	   CALL(cpu, adress_to_pc);
    else
        cpu->registers.pc += 3;
}

void CPE(Cpu8080 *cpu)
{
    uint16_t adress_to_pc = read_byte_address(cpu);

    // if Parity bit is true, then
    if (cpu->registers.F.p)
	   CALL(cpu, adress_to_pc);
    else
        cpu->registers.pc += 3;
}

void RET(Cpu8080 *cpu)
{
    /**
     * SP+1 ->  Higher
     * SP   ->  Lower
     */

    uint8_t     *memory = cpu->memory;
    uint16_t    *SP = &cpu->registers.sp;

    unsigned int  Lower   = memory[*SP];
	unsigned int  Higher  = memory[*SP+1];

	*SP+=2;

    cpu->registers.pc = (Higher) | Lower << 8;
}

void RZ (Cpu8080 *cpu)
{ 

    // if Zero bit is true, then
    if (cpu->registers.F.z)
		RET(cpu);
    else
        cpu->registers.pc += 1;
}

void RNZ (Cpu8080 *cpu)
{

    // if Zero bit is false, then
    if (! cpu->registers.F.z)
		RET(cpu);
    else
        cpu->registers.pc += 1;    
}

void RNC (Cpu8080 *cpu)
{ 

    // if Carry bit is false, then
    if (! (cpu->registers.F.cy))
		RET(cpu);
    else
        cpu->registers.pc += 1;
}

void RC (Cpu8080 *cpu)
{   

    // if Carry bit is true, then
    if (cpu->registers.F.cy)
		RET(cpu);
    else
        cpu->registers.pc += 1;    
}

void RP (Cpu8080 *cpu)
{   
    // if Parity bit is true, then
    if (cpu->registers.F.p)
		RET(cpu);
    else
        cpu->registers.pc += 1;
}

void RPO (Cpu8080 *cpu)
{   
    // if Parity bit is false, then
    if (! cpu->registers.F.p)
		RET(cpu);
    else
        cpu->registers.pc += 1;
}

void RPE (Cpu8080 *cpu)
{   
    // if Parity bit is true, then
    if (cpu->registers.F.p)
		RET(cpu);
    else
        cpu->registers.pc += 1;
}

void RM (Cpu8080 *cpu)
{    

    // if sign bit is true, then
    if (cpu->registers.F.s)
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

void load_rom(Cpu8080 *cpu)
{
    char* rom = get_rom();
    cpu->rom = calloc(get_rom_size() + 0x100, sizeof(char));
    
    for (int i = 0; i < get_rom_size(); i++)
    {
        cpu->rom[i+0x100] = rom[i];

    }

    if (cpu->rom == NULL) {
        fprintf(stderr, "Failed to load ROM\n");
        exit(1);
        return;
    }

    //printf("rom loaded suscefuly");
}

void load_rom_to_memory(Cpu8080 *cpu) 
{
    size_t rom_size = get_rom_size();
    for (size_t i = 0; i <  rom_size; i++)
    {
        cpu->memory[i] = cpu->rom[i];
    }
}

void emulate_instruction(Cpu8080 *cpu) 
{
    bool mudou = false;
    if (cpu->registers.pc == 0 && mudou == false)
    {
        cpu->registers.pc = 0x100;
    }
    if (cpu->registers.pc == 0 && mudou == true)
    {
        printf("terminou\n");
        exit(0);
    }

    if (cpu->registers.pc == 0x05)
    {
        uint16_t HL = twoU8_to_u16value(*H, *L);
        printf("HL: %u\n", HL);
        if (cpu->registers.C == 9)
        {
            for (int i = HL; cpu->memory[i] != '$'; i++)
            {
                printf("%c", cpu->memory[i]);
            }
            printf("\n");
        }
        else if (cpu->registers.E == 2)
        {
            printf("E");
        }
    }

    uint8_t instruction = cpu->rom[cpu->registers.pc];

    switch (instruction) {
        case 0x00: case 0x08: case 0x10: case 0x20: case 0x30:
            NOP(cpu);
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
            DCX(cpu, &cpu->registers.B, &cpu->registers.C);
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
            INX(cpu, &cpu->registers.D, &cpu->registers.E);
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
    
		case 0x17:
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
			INX(cpu, &cpu->registers.H, &cpu->registers.L);
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
            DCX(cpu, &cpu->registers.H, &cpu->registers.L);
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
            cpu->registers.sp = twoU8_to_u16adress(
                cpu->rom[cpu->registers.pc+1], 
                cpu->rom[cpu->registers.pc+2]
                );
            
            cpu->registers.pc+=3;

            break;

        case 0x32:
            STA(cpu);
            break;

        case 0x33:
            INX_16(cpu, &cpu->registers.sp);
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
            LDA(cpu);
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
            MOV_im_to_reg(cpu, &cpu->registers.A);
            break;

		case 0x3F:
			CMC(cpu);
			break;

        // MOVs
        case 0x40:
            MOV_reg_to_reg(cpu,B, B);
            break;

        case 0x41:
            MOV_reg_to_reg(cpu,B, C);
            break;

        case 0x42:
            MOV_reg_to_reg(cpu,B, D);
            break;

        case 0x43:
            MOV_reg_to_reg(cpu,B, E);
            break;

        case 0x44:
            MOV_reg_to_reg(cpu,B, H);
            break;

        case 0x45:
            MOV_reg_to_reg(cpu,B, L);
            break;

        case 0x46:
            MOV_mem_to_reg(cpu, B);
            break;

        case 0x47:
            MOV_reg_to_reg(cpu,B, A);
            break;

        case 0x48:
            MOV_reg_to_reg(cpu,C, B);
            break;

        case 0x49:
            MOV_reg_to_reg(cpu,C, C);
            break;

        case 0x4A:
            MOV_reg_to_reg(cpu,C, D);
            break;

        case 0x4B:
            MOV_reg_to_reg(cpu,C, E);
            break;

        case 0x4C:
            MOV_reg_to_reg(cpu,C, H);
            break;

        case 0x4D:
            MOV_reg_to_reg(cpu,C, L);
            break;

        case 0x4E:
            MOV_mem_to_reg(cpu, C);
            break;

        case 0x4F:
            MOV_reg_to_reg(cpu,C, A);
            break;

        case 0x50:
            MOV_reg_to_reg(cpu,D, B);
            break;

        case 0x51:
            MOV_reg_to_reg(cpu,D, C);
            break;

        case 0x52:
            MOV_reg_to_reg(cpu,D, D);
            break;

        case 0x53:
            MOV_reg_to_reg(cpu,D, E);
            break;

        case 0x54:
            MOV_reg_to_reg(cpu,D, H);
            break;

        case 0x55:
            MOV_reg_to_reg(cpu,D, L);
            break;

        case 0x56:
            MOV_mem_to_reg(cpu, D);
            break;

        case 0x57:
            MOV_reg_to_reg(cpu,D, A);
            break;

        case 0x58:
            MOV_reg_to_reg(cpu,E, B);
            break;

        case 0x59:
            MOV_reg_to_reg(cpu,E, C);
            break;

        case 0x5A:
            MOV_reg_to_reg(cpu,E, D);
            break;

        case 0x5B:
            MOV_reg_to_reg(cpu,E, E);
            break;

        case 0x5C:
            MOV_reg_to_reg(cpu,E, H);
            break;

        case 0x5D:
            MOV_reg_to_reg(cpu,E, L);
            break;

        case 0x5E:
            MOV_mem_to_reg(cpu, E);
            break;

        case 0x5F:
            MOV_reg_to_reg(cpu,E, A);
            break;

        case 0x60:
            MOV_reg_to_reg(cpu,H, B);
            break;

        case 0x61:
            MOV_reg_to_reg(cpu,H, C);
            break;

        case 0x62:
            MOV_reg_to_reg(cpu,H, D);
            break;

        case 0x63:
            MOV_reg_to_reg(cpu,H, E);
            break;

        case 0x64:
            MOV_reg_to_reg(cpu,H, H);
            break;

        case 0x65:
            MOV_reg_to_reg(cpu,H, L);
            break;

        case 0x66:
            MOV_mem_to_reg(cpu, H); 
            break;

        case 0x67:
            MOV_reg_to_reg(cpu,H, A);
            break;

        case 0x68:
            MOV_reg_to_reg(cpu,L, B);
            break;

        case 0x69:
            MOV_reg_to_reg(cpu,L, C);
            break;

        case 0x6A:
            MOV_reg_to_reg(cpu,L, D);
            break;

        case 0x6B:
            MOV_reg_to_reg(cpu,L, E);
            break;

        case 0x6C:
            MOV_reg_to_reg(cpu,L, H);
            break;

        case 0x6D:
            MOV_reg_to_reg(cpu,L, L);
            break;

        case 0x6E:
            MOV_mem_to_reg(cpu, L);
            break;

        case 0x6F:
            MOV_reg_to_reg(cpu,L, A);
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
            MOV_reg_to_reg(cpu,A, B);
            break;

        case 0x79:
            MOV_reg_to_reg(cpu,A, C);
            break;

        case 0x7a:
            MOV_reg_to_reg(cpu,A, D);
            break;

        case 0x7b:
            MOV_reg_to_reg(cpu,A, E);
            break;

        case 0x7c:
            MOV_reg_to_reg(cpu,A, H);
            break;

        case 0x7d:
            MOV_reg_to_reg(cpu,A, L);
            break;

        case 0x7e:
            MOV_mem_to_reg(cpu, A);
            break;

        case 0x7f:
            MOV_reg_to_reg(cpu, A, A);
            break;

        // ADDs
        case 0x80:
            ADD(cpu, *B);
            break;

        case 0x81:
            ADD(cpu, *C);
            break;

        case 0x82:
            ADD(cpu, *D);
            break;

        case 0x83:
            ADD(cpu, *E);
            break;

        case 0x84:
            ADD(cpu, *H);
            break;

        case 0x85:
            ADD(cpu, *L);
            break;

        case 0x86:
        {
            uint16_t mem_adress = twoU8_to_u16value(cpu->registers.H, cpu->registers.L); 
            uint8_t value = cpu->memory[mem_adress];

            ADD(cpu, value);
            break;
        }

        case 0x87:
            ADD(cpu, *A);
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
            uint16_t mem_adress = twoU8_to_u16value(cpu->registers.H, cpu->registers.L); 
            uint8_t value = cpu->memory[mem_adress];

            ADC(cpu, &value);
            break;
        }

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
            uint16_t mem_adress = twoU8_to_u16value(cpu->registers.H, cpu->registers.L); 
            uint8_t value = cpu->memory[mem_adress];

            SUB(cpu, &value);
            break;
        }

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
                uint16_t mem_adress = twoU8_to_u16value(cpu->registers.H, cpu->registers.L); 
                uint8_t value = cpu->memory[mem_adress];

                SBB(cpu, &value);
                break;
            }

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
            uint16_t mem_adress = twoU8_to_u16value(cpu->registers.H, cpu->registers.L); 
            uint8_t value = cpu->memory[mem_adress];

            ANA(cpu, &value);
            break;
        }

        case 0xA7:
            ANA(cpu, A);
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
            uint16_t mem_adress = twoU8_to_u16value(cpu->registers.H, cpu->registers.L); 
            uint8_t value = cpu->memory[mem_adress];

            XRA(cpu, &value);
            break;
        }

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
            uint16_t mem_adress = twoU8_to_u16value(cpu->registers.H, cpu->registers.L); 
            uint8_t value = cpu->memory[mem_adress];

            ORA(cpu, &value);
            break;
        }

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
            uint16_t mem_adress = twoU8_to_u16value(cpu->registers.H, cpu->registers.L); 
            uint8_t value = cpu->memory[mem_adress];

            CMP(cpu, &value);
            break;
        }

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
            printf("OUT\n");
    		break;

    	case 0xD4:
    		CNC(cpu);
    		break;

        case 0xD5:
    		PUSH(cpu, &cpu->registers.D, &cpu->registers.E);
    		break;

        case 0xD6:
            SUI(cpu);
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
            printf("IN\n");
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
        
        case 0xE0:
    		RPO(cpu);
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

        case 0xE4:
    		CPO(cpu);
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

        case 0xE8:
    		RPE(cpu);
    		break;
    	
    	case 0xE9:
    		PCHL(cpu);
    		break;

        case 0xEA:
    		JPE(cpu);
    		break;

        case 0xEB:	
    		XCHG(cpu);
    		break;

        case 0xEC:	
    		CPE(cpu);
    		break;
    	
    	case 0xEE:
            XRI(cpu);
            break;

    	case 0xEF:
    		RST(cpu, 0x28);
    		break;
    	
    	case 0xF0:
    		RP(cpu);
    		break;

    	case 0xF1:
    		POP_PSW(cpu);
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
    		PUSH_PSW(cpu);	
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
            CPI(cpu);
            break;

    	case 0xFF:
    		RST(cpu, 0x38);
    		break;

        default:
    		printf("Unimplemented instruction: 0x%02X\n", instruction);
            cpu->registers.pc++;
            break;
    }
}

void intel8080_main(Cpu8080 *cpu)
{
    
    load_rom(cpu);
    cpu->rom[0x0005] = (char)0xc9;
    //load_rom_to_memory(cpu);
    
    video_buffer_to_screen(cpu);
    update_screen();

    /* Main */
    SDL_Event event;
    int running = 1;  /* Flag to control loop execution       */
    while (running)
    {
        #ifndef CPU_USE_POOL_IN_CODE
        /* Get events from SDL */
        while (SDL_PollEvent(&event))
        {
            /* IF event is to close window, then stop to run */
            if (event.type == SDL_QUIT)
            {
                running = 0;
                exit(0);
            }
        }
        #endif

        emulate_instruction(cpu);
        video_buffer_to_screen(cpu);
        update_screen();

    }
}