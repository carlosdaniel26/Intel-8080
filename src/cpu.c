#include <stdint.h>
#include <stdio.h>
#include <SDL2/SDL.h>
#include <errno.h>
#include <unistd.h>
#include <sys/types.h>

#include <cpu.h>
#include <helper.h>
#include <debug.h>
#include <rom.h>
#include <screen.h>
#include <main.h>

unsigned int rom_size;
int8_t error_occurred = -1;

// Global alias
uint8_t* A;
uint8_t* B;
uint8_t* C;
uint8_t* D;
uint8_t* E;
uint8_t* H;
uint8_t* L;

void timer_irq(Cpu8080 *cpu)
{
	if (cpu->cycles % TIMER_INTERRUPT_CYCLES == 0)
	{
		cpu->memory[ISRDELAY]--;
	}
}

void vblank_irq(Cpu8080 *cpu)
{
	if (cpu->cycles % VBLANK_INTERRUPT_CYCLES == 0)
	{
		buffer_to_screen(cpu);
		update_screen();
	}
}

Cpu8080* init_cpu() 
{
	Cpu8080 *cpu = (Cpu8080*)malloc(sizeof(Cpu8080));

	if (!cpu) {
		fprintf(stderr, "Error allocating memory for CPU: %s\n", strerror(errno));
		exit(EXIT_FAILURE);
	}

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

	cpu->registers.sp = 0x2400;
	cpu->registers.pc = 0x00;

	cpu->interrupt_enabled = false;
	
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
	int parity = 0;

	int i;
	for (i = 0; i < size; i++)
	{
		parity += x & 1;
		x = x >> 1;
	}
	return (parity % 2 == 0);
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
	cpu->registers.F.ac = (res > 0x09);
}

void BcdArithFlags(Cpu8080 *cpu, uint16_t res)
{
	cpu->registers.F.cy = (res > 0xff);
	cpu->registers.F.z = ((res & 0xff) == 0);
	cpu->registers.F.s = (0x80 == (res & 0x80));
	cpu->registers.F.p = parity(res & 0xff, 8);
	cpu->registers.F.ac = (res > 0x09);
}

void NOP(Cpu8080 *cpu) 
{
	cpu->registers.pc++;
	return;
}

void LXI(Cpu8080 *cpu, uint8_t *msgReg, uint8_t *lsbReg) 
{
	*lsbReg = cpu->memory[cpu->registers.pc + 1];
	*msgReg = cpu->memory[cpu->registers.pc + 2];
	cpu->registers.pc += 3;
}

void LDA(Cpu8080 *cpu)
{
	uint16_t address = read_byte_address(cpu);
	cpu->registers.A = cpu->memory[address];

	cpu->registers.pc += 3;
}

void LDAX(Cpu8080 *cpu, uint8_t *msbReg, uint8_t *lsbReg)
{
	uint16_t address = (*msbReg << 8) | *lsbReg;

	cpu->registers.A = cpu->memory[address];

	cpu->registers.pc++;
}

void STAX(Cpu8080 *cpu, uint8_t *_register1, uint8_t *_register2)
{
	uint16_t adress = (*_register1 << 8) | *_register2;
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

void ADC(Cpu8080 *cpu, uint8_t value)
{
	uint16_t result16 = (uint16_t)value + (uint16_t)cpu->registers.A + (uint16_t)cpu->registers.F.cy;

	BcdArithFlags(cpu, result16);

	cpu->registers.A = result16 & 0xFF;

	cpu->registers.pc++;
}

void ACI(Cpu8080 *cpu)
{
	uint8_t value = read_byte(cpu);

	uint16_t result16 = value + cpu->registers.A + (cpu->registers.F.cy);

	BcdArithFlags(cpu, result16);

	cpu->registers.A = result16 & 0xFF;

	cpu->registers.pc += 2;
}

void SBI(Cpu8080 *cpu)
{
	uint8_t value = read_byte(cpu);

	uint16_t result16 = cpu->registers.A - value - (cpu->registers.F.cy);

	BcdArithFlags(cpu, result16);

	cpu->registers.A = result16 & 0xFF;

	cpu->registers.pc += 2;
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

void SUB(Cpu8080 *cpu, uint8_t value)
{
	uint16_t result16 = (uint16_t)cpu->registers.A - (uint16_t)value;

	BcdArithFlags(cpu, result16);

	cpu->registers.A = result16 & 0xFF;

	cpu->registers.pc++;
}

void SBB(Cpu8080 *cpu, uint8_t value)
{
	uint16_t result16 = (uint16_t)cpu->registers.A - (uint16_t)value - (uint16_t)cpu->registers.F.cy;

	BcdArithFlags(cpu, result16);

	cpu->registers.A = result16 & 0xFF;

	cpu->registers.pc++;
}

void ANA(Cpu8080 *cpu, uint8_t value)
{
	cpu->registers.A = cpu->registers.A & value;
	
	BcdArithFlags(cpu, (uint16_t)cpu->registers.A);
	
	cpu->registers.F.cy = 0;

	cpu->registers.pc++;
}

void ANI(Cpu8080 *cpu)
{
	uint8_t value = read_byte(cpu);

	ANA(cpu, value);

	cpu->registers.pc++;
}

void XRA(Cpu8080 *cpu, uint8_t value)
{
	uint16_t result16 = (uint16_t)cpu->registers.A ^ (uint16_t)value;

	BcdArithFlags(cpu, result16);
	cpu->registers.F.cy = 0;
	cpu->registers.F.ac = 0;

	cpu->registers.A = result16;

	cpu->registers.pc += 1;
}

void XRI(Cpu8080 *cpu)
{
	uint8_t value = read_byte(cpu);

	XRA(cpu, value);

	cpu->registers.pc++;
}

void ORA(Cpu8080 *cpu, uint8_t *_register)
{
	uint16_t result16 = cpu->registers.A | *_register;
	BcdArithFlags(cpu, result16);

	cpu->registers.F.cy = 0;
	cpu->registers.F.ac = 0;
	cpu->registers.A = result16;

	cpu->registers.pc++;
}

void ORI(Cpu8080 *cpu)
{
	uint8_t value = read_byte(cpu);

	uint16_t result16 = cpu->registers.A | value;
	BcdArithFlags(cpu, result16);

	cpu->registers.F.cy = 0;
	cpu->registers.F.ac = 0;
	cpu->registers.A = result16;

	cpu->registers.pc++;
}

void CMP(Cpu8080 *cpu, uint8_t value) 
{
	uint16_t result16 = cpu->registers.A - value;

	cpu->registers.F.z = (result16 == 0);
	cpu->registers.F.s = (0x80 == (result16 & 0x80));
	cpu->registers.F.p = parity(result16, 8);
	cpu->registers.F.cy = (cpu->registers.A < value);

	cpu->registers.pc++;
}

void DCX(Cpu8080 *cpu, uint8_t *msbReg, uint8_t *lsbReg) 
{
	(*lsbReg)--;

	if (*lsbReg == 0xFF)
	{
		(*msbReg)--;
	}

	cpu->registers.pc++;
}

void DCR(Cpu8080 *cpu, uint8_t *_register)
{
	uint16_t result16 = *_register - 1;

	cpu->registers.F.z = (result16 == 0);
	cpu->registers.F.s = (0x80 == (result16 & 0x80));
	cpu->registers.F.p = parity(result16, 8);

	*_register = (result16 & 0xFF);

	cpu->registers.pc++;
}

void INX(Cpu8080 *cpu, uint8_t *msbReg, uint8_t *lsbReg) 
{
	(*lsbReg)++;
	// overflow
	if (*lsbReg == 0)
	{
		(*msbReg)++;
	}

	cpu->registers.pc++;
}

void INR(Cpu8080 *cpu, uint8_t *_register)
{
	uint16_t result16 = (uint16_t)*_register + 1;
	*_register = (result16 & 0xFF);

	ArithFlagsA(cpu, result16);

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

	CMP(cpu, value);
	cpu->registers.pc++;
}

void CMA(Cpu8080 *cpu)
{
	cpu->registers.A = ~(cpu->registers.A);

	cpu->registers.pc += 1;
}

void DAA(Cpu8080 *cpu)
{
	uint8_t fourLSB = ((cpu->registers.A << 4) >> 4);
	if ((fourLSB > 0x09) || (cpu->registers.F.ac == 1))
	{
		uint16_t resultLSB  = fourLSB + 0x06;
		cpu->registers.A    = cpu->registers.A + 0x06;
		BcdArithFlags(cpu, resultLSB);
	}

	uint8_t fourMSB = (cpu->registers.A >> 4);
	if ((fourMSB > 0x09) || (cpu->registers.F.cy == 1))
	{
		uint16_t resultMSB = fourMSB + 0x06;
		cpu->registers.A = cpu->registers.A + 0x06;
		BcdArithFlags(cpu, resultMSB);
	}
}

void DAD(Cpu8080 *cpu, uint32_t register_pair) 
{
	uint32_t HL = (cpu->registers.H << 8) | cpu->registers.L;
	uint32_t result = HL + register_pair;

	cpu->registers.F.cy = (result > 0xFFFF);

	cpu->registers.H = (uint8_t)(result >> 8);
	cpu->registers.L = (uint8_t)(result & 0xFF);

	cpu->registers.pc += 1;
}

void ADD(Cpu8080 *cpu, uint8_t value)
{
	uint16_t result16 = (uint16_t)value + (uint16_t)cpu->registers.A;
	uint8_t result8 = result16 & 0xFF;

	cpu->registers.A = result8;

	BcdArithFlags(cpu, result16);

	cpu->registers.pc++;
}

void ADI(Cpu8080 *cpu)
{
	uint8_t value = read_byte(cpu);

	uint16_t result16 = value + cpu->registers.A;
	uint8_t result8 = result16 & 0xFF;

	cpu->registers.A = result8;

	BcdArithFlags(cpu, result16);

	cpu->registers.pc += 2;
}

void RLC(Cpu8080 *cpu)
{
	uint8_t temp = cpu->registers.A;

	cpu->registers.A = temp << 1 | temp >> 7;   

	cpu->registers.F.cy = (temp >> 7) > 0;

	cpu->registers.pc += 1;
}

void RRC(Cpu8080 *cpu)
{
	uint8_t temp = cpu->registers.A;

	cpu->registers.A = temp >> 1 | temp << 7;

	cpu->registers.F.cy = (cpu->registers.A >> 7) > 0;

	cpu->registers.pc+=1;    
}

void RAR(Cpu8080 *cpu)
{
	uint8_t temp = cpu->registers.A;
	uint8_t msb  = (cpu->registers.A >> 7) << 7;
	cpu->registers.A = (temp >> 1) | msb;
	
	cpu->registers.F.cy = (temp << 7) >> 7;

	cpu->registers.pc+=1;
}

void RAL(Cpu8080 *cpu)
{
	uint8_t temp = cpu->registers.A;
	uint8_t msb  = (cpu->registers.A >> 7);
	
	cpu->registers.A    = (temp << 1) | (cpu->registers.F.cy);
	cpu->registers.F.cy = msb;

	cpu->registers.pc+=1;
	
}

void SHLD(Cpu8080 *cpu)
{
	uint16_t adress = read_byte_address(cpu);

	cpu->memory[adress]     = cpu->registers.L;
	cpu->memory[adress + 1] = cpu->registers.H;

	cpu->registers.pc += 3;
}

void CMC(Cpu8080 *cpu)
{
	cpu->registers.F.cy = ~cpu->registers.F.cy;

	cpu->registers.pc+=1;
}

void POP(Cpu8080 *cpu, uint8_t *msbReg, uint8_t *lsbReg)
{
	uint16_t sp = cpu->registers.sp;

	*lsbReg = cpu->memory[sp];
	*msbReg = cpu->memory[sp + 1];

	cpu->registers.sp += 2;
	cpu->registers.pc += 1;
}

void POP_PSW(Cpu8080 *cpu)
{
	uint16_t sp = cpu->registers.sp;

	uint8_t PSW = cpu->memory[sp];

	// carry flag (CY) <- ((SP))_0
	cpu->registers.F.cy = ((PSW & 0x1) != 0);

	// parity flag (P) <- ((SP))_2
	cpu->registers.F.p = ((PSW & 0x4) != 0);

	// auxiliary flag (AC) <- ((SP))_4
	cpu->registers.F.ac = ((PSW & 0x10) != 0);

	// zero flag (Z) <- ((SP))_6
	cpu->registers.F.z = ((PSW & 0x40) != 0);

	// sign flag (S) <- ((SP))_7
	cpu->registers.F.s = ((PSW & 0x80) != 0);


	
	cpu->registers.A = cpu->memory[sp+1];

	cpu->registers.sp += 2;
	cpu->registers.pc+=1;
}

void PUSH(Cpu8080 *cpu, uint8_t msbReg, uint8_t lsbReg)
{
	uint16_t sp = cpu->registers.sp;

	cpu->memory[sp-1] = msbReg;
	cpu->memory[sp-2] = lsbReg;
	
	cpu->registers.sp -= 2;
	cpu->registers.pc +=1 ;
}

void PUSH_PSW(Cpu8080 *cpu)
{
	uint16_t sp = cpu->registers.sp;

	cpu->memory[sp - 2] =   (cpu->registers.F.cy & 0x01) |  // 0th
							(0x02) |                        // 1st
							(cpu->registers.F.cy << 2) |    // 2nd
							(cpu->registers.F.ac << 4) |    // 4th
							(cpu->registers.F.z << 6) |     // 6th
							(cpu->registers.F.s << 7) |     // th
							(0x00);                         // 0 in rest
	
	cpu->memory[sp - 1] = cpu->registers.A;
	cpu->registers.sp -= 2;

	cpu->registers.pc += 1;
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
	uint16_t HL = twoU8_to_u16value(cpu->registers.H, cpu->registers.L);
	uint16_t *SP = &cpu->registers.sp;

	*SP = HL;

	cpu->registers.pc+=1;
}

void PCHL(Cpu8080 *cpu)
{
	uint16_t HL = twoU8_to_u16value(cpu->registers.H, cpu->registers.L);
			 
	cpu->registers.pc = HL; 
}

void XTHL(Cpu8080 *cpu)
{
	uint16_t sp = cpu->registers.sp;

	uint8_t temp_l = cpu->registers.L;
	uint8_t temp_h = cpu->registers.H;

	cpu->registers.L = cpu->memory[sp];
	cpu->registers.H = cpu->memory[sp + 1];

	cpu->memory[sp] = temp_l;
	cpu->memory[sp + 1] = temp_h;

	cpu->registers.pc += 1;
}

void CALL(Cpu8080 *cpu, unsigned int adress)
{
	unsigned int *PC = &cpu->registers.pc;
	uint16_t     *SP = &cpu->registers.sp;
	
	uint8_t Higher   = (*PC+3) >> 8;
	uint8_t Lower  = (*PC+3) & 0xff;
	
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

void RST(Cpu8080* cpu, uint16_t id)
{
	uint16_t new_pc_position = id * 0x08;

	CALL(cpu, new_pc_position);
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

	unsigned int  Lower   = memory[*SP+1];
	unsigned int  Higher  = memory[*SP];

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
	
}

void IN(Cpu8080* cpu)
{
	// uint8_t port = cpu->rom[((*PC) + 1)];
	
	// get input like a getch
	// cpu->registers.A = machineIN() 
	
	cpu->registers.pc += 2;
}

void OUT(Cpu8080 *cpu)
{
	// unsigned int *PC = &cpu->registers.pc;
	// uint8_t port = cpu->rom[((*PC) + 1)];
	
	// MachineOUT()
	cpu->registers.pc += 2;	
}

static inline void load_rom(Cpu8080 *cpu)
{
	char* rom = get_rom();
	cpu->rom = calloc(get_rom_size(), sizeof(char));
	

	if (cpu->rom == NULL) {
		fprintf(stderr, "Failed to load ROM\n");
		exit(EXIT_FAILURE);
		return;
	}

	for (int i = 0; i < get_rom_size(); i++)
	{
		cpu->rom[i] = rom[i];

	}
}

static inline void load_rom_to_memory(Cpu8080 *cpu) 
{
	size_t rom_size = get_rom_size();
	for (size_t i = 0; i <  rom_size; i++)
	{
		cpu->memory[i] = cpu->rom[i];
	}
}

static inline uint8_t emulate_instruction(Cpu8080 *cpu)
{
	uint8_t instruction = cpu->rom[cpu->registers.pc];
	uint16_t address = (cpu->registers.H << 8) | (cpu->registers.L);

	//uint16_t bc = (cpu->registers.B << 8) | cpu->registers.C;

	FILE *file = fopen("pc_log.txt", "a");
	if (file != NULL)
		fprintf(file, "PC = 0x%04X\nA = 0x%04X\n", cpu->registers.pc, *A);

	fclose(file);

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

		case 0x06: /*MVI B,D8 */
			cpu->registers.B = cpu->rom[cpu->registers.pc + 1];
			cpu->registers.pc += 2;
			break;

		case 0x07:
			RLC(cpu);
			break;

		case 0x09:
		{
			uint32_t bc = (cpu->registers.B << 8) | cpu->registers.C;
			DAD(cpu, bc);
			break;
		}

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
			cpu->registers.C = cpu->rom[cpu->registers.pc + 1];
			cpu->registers.pc += 2;
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
			*D = cpu->rom[cpu->registers.pc + 1];
			cpu->registers.pc += 2;
			break;
	
		case 0x17:
			RAL(cpu);		
			break;

		case 0x19:
		{
			uint32_t de = (cpu->registers.D << 8) | cpu->registers.E;
			DAD(cpu, de);
			break;
		}

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
			cpu->registers.E = cpu->rom[cpu->registers.pc + 1];
			cpu->registers.pc+=2;
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
			*H = cpu->rom[cpu->registers.pc + 1];
			cpu->registers.pc += 2;
			break;

		case 0x27:
			DAA(cpu);
			break;

		case 0x29:
		{
			uint32_t hl = (cpu->registers.H << 8) | cpu->registers.L;
			DAD(cpu, hl);
			break;
		}

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
			*L = cpu->rom[cpu->registers.pc + 1];
			cpu->registers.pc+=2;
			break;

		case 0x2f:
			CMA(cpu);
			break;


		case 0x31:
		{
			uint16_t PC = cpu->registers.pc;
			cpu->registers.sp = (cpu->rom[PC + 2] << 8) | cpu->rom[PC + 1];
			
			cpu->registers.pc+=3;

			break;
		}

		case 0x32:
			STA(cpu);
			break;

		case 0x33:
			cpu->registers.sp++;
			break;

		case 0x34:
			{
				INR(cpu, &cpu->memory[address]);
				cpu->registers.pc+=2;
			}
			break;

		case 0x35:
				DCR(cpu, &cpu->memory[address]);
			break;

		case 0x36:
		{
			uint8_t value = read_byte(cpu);
			cpu->memory[address] = value;
			cpu->registers.pc+=2;	
		}
			break;

		case 0x37:
			STC(cpu);
			break;

		case 0x39:
			DAD(cpu, cpu->registers.sp);
			break;

		case 0x3A:
			LDA(cpu);
			break;

		case 0x3B:
			cpu->registers.sp--;
			cpu->registers.pc++;
			break;

		case 0x3C:
			INR(cpu, &cpu->registers.A);
			break;

		case 0x3D:
			DCR(cpu, &cpu->registers.A);
			break;

		case 0x3E:
		{
			uint8_t value = read_byte(cpu);
			cpu->registers.A = value;
			cpu->registers.pc += 2;
			break;
		}

		case 0x3F:
			CMC(cpu);
			break;

		case 0x40:
			*B = *B;
			cpu->registers.pc += 1;
			break;
		case 0x41:
			*B = *C;
			cpu->registers.pc += 1;
			break;
		case 0x42:
			*B = *D;
			cpu->registers.pc += 1;
			break;
		case 0x43:
			*B = *E;
			cpu->registers.pc += 1;
			break;
		case 0x44:
			*B = *H;
			cpu->registers.pc += 1;
			break;
		case 0x45:
			*B = *L;
			cpu->registers.pc += 1;
			break;
		case 0x46:
			*B = cpu->memory[address];
			cpu->registers.pc += 1;
			break;
		case 0x47:
			*B = *A;
			cpu->registers.pc += 1;
			break;
		case 0x48:
			*C = *B;
			cpu->registers.pc += 1;
			break;
		case 0x49:
			*C = *C;
			cpu->registers.pc += 1;
			break;
		case 0x4A:
			*C = *D;
			cpu->registers.pc += 1;
			break;
		case 0x4B:
			*C = *E;
			cpu->registers.pc += 1;
			break;
		case 0x4C:
			*C = *H;
			cpu->registers.pc += 1;
			break;
		case 0x4D:
			*C = *L;
			cpu->registers.pc += 1;
			break;
		case 0x4E:
			*C = cpu->memory[address];
			cpu->registers.pc += 1;
			break;
		case 0x4F:
			*C = *A;
			cpu->registers.pc += 1;
			break;
		case 0x50:
			*D = *B;
			cpu->registers.pc += 1;
			break;
		case 0x51:
			*D = *C;
			cpu->registers.pc += 1;
			break;
		case 0x52:
			*D = *D;
			cpu->registers.pc += 1;
			break;
		case 0x53:
			*D = *E;
			cpu->registers.pc += 1;
			break;
		case 0x54:
			*D = *H;
			cpu->registers.pc += 1;
			break;
		case 0x55:
			*D = *L;
			cpu->registers.pc += 1;
			break;
		case 0x56:
			*D = cpu->memory[address];
			cpu->registers.pc += 1;
			break;
		case 0x57:
			*D = *A;
			cpu->registers.pc += 1;
			break;

		case 0x58:
			cpu->registers.E = cpu->registers.B;
			cpu->registers.pc += 1;
			break;

		case 0x59:
			cpu->registers.E = cpu->registers.C;
			cpu->registers.pc += 1;
			break;

		case 0x5a:
			cpu->registers.E = cpu->registers.D;
			cpu->registers.pc += 1;
			break;

		case 0x5b:
			cpu->registers.E = cpu->registers.E;
			cpu->registers.pc += 1;
			break;

		case 0x5c:
			cpu->registers.E = cpu->registers.H;
			cpu->registers.pc += 1;
			break;

		case 0x5d:
			cpu->registers.E = cpu->registers.L;
			cpu->registers.pc += 1;
			break;

		case 0x5e:
			cpu->registers.E = cpu->memory[address];
			cpu->registers.pc += 1;
			break;

		case 0x5f:
			cpu->registers.E = cpu->registers.A;
			cpu->registers.pc += 1;
			break;

		case 0x60:
			cpu->registers.H = cpu->registers.B;
			cpu->registers.pc += 1;
			break;

		case 0x61:
			cpu->registers.H = cpu->registers.C;
			cpu->registers.pc += 1;
			break;

		case 0x62:
			cpu->registers.H = cpu->registers.D;
			cpu->registers.pc += 1;
			break;
			
		case 0x63:
			cpu->registers.H = cpu->registers.E;
			cpu->registers.pc += 1;
			break;

		case 0x64:
			cpu->registers.H = cpu->registers.H;
			cpu->registers.pc += 1;
			break;

		case 0x65:
			cpu->registers.H = cpu->registers.L;
			cpu->registers.pc += 1;
			break;

		case 0x66:
			cpu->registers.H = cpu->memory[address];
			cpu->registers.pc += 1;
			break;

		case 0x67:
			cpu->registers.H = cpu->registers.A;
			cpu->registers.pc += 1;
			break;

		case 0x68:
			cpu->registers.L = cpu->registers.B;
			cpu->registers.pc += 1;
			break;

		case 0x69:
			cpu->registers.L = cpu->registers.C;
			cpu->registers.pc += 1;
			break;

		case 0x6a:
			cpu->registers.L = cpu->registers.D;
			cpu->registers.pc += 1;
			break;

		case 0x6b:
			cpu->registers.L = cpu->registers.E;
			cpu->registers.pc += 1;
			break;

		case 0x6c:
			cpu->registers.L = cpu->registers.H;
			cpu->registers.pc += 1;
			break;

		case 0x6d:
			cpu->registers.L = cpu->registers.L;
			cpu->registers.pc += 1;
			break;

		case 0x6e:
			cpu->registers.L = cpu->memory[address];
			cpu->registers.pc += 1;
			break;

		case 0x6f:
			cpu->registers.L = cpu->registers.A;
			cpu->registers.pc += 1;
			break;

		case 0x70:
			cpu->memory[address] = cpu->registers.B;
			cpu->registers.pc += 1;
			break;

		case 0x71:
			cpu->memory[address] = cpu->registers.C;
			cpu->registers.pc += 1;
			break;

		case 0x72:
			cpu->memory[address] = cpu->registers.D;
			cpu->registers.pc += 1;
			break;

		case 0x73:
			cpu->memory[address] = cpu->registers.E;
			cpu->registers.pc += 1;
			break;

		case 0x74:
			cpu->memory[address] = cpu->registers.H;
			cpu->registers.pc += 1;
			break;

		case 0x75:
			cpu->memory[address] = cpu->registers.L;
			cpu->registers.pc += 1;
			break;

		case 0x76:
			HLT();
			break;

		case 0x77:
			cpu->memory[address] = *A;
			cpu->registers.pc += 1;
			break;

		 case 0x78:
			*A = *B;
			cpu->registers.pc += 1;
			break;

		case 0x79:
			*A = *C;
			cpu->registers.pc += 1;
			break;

		case 0x7A:
			*A = *D;
			cpu->registers.pc += 1;
			break;

		case 0x7B:
			*A = *E;
			cpu->registers.pc += 1;
			break;

		case 0x7C:
			*A = *H;
			cpu->registers.pc += 1;
			break;

		case 0x7D:
			*A = *L;
			cpu->registers.pc += 1;
			break;

		case 0x7E:
			*A = cpu->memory[address];
			cpu->registers.pc += 1;
			break;
			
		case 0x7F:
			*A = *A;
			cpu->registers.pc += 1;
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
			uint8_t value = cpu->memory[address];

			ADD(cpu, value);
			break;
		}

		case 0x87:
			ADD(cpu, *A);
			break;

		// ADCs
		case 0x88:
			ADC(cpu, *B);
			break;

		case 0x89:
			ADC(cpu, *C);
			break;

		case 0x8a:
			ADC(cpu, *D);
			break;

		case 0x8b:
			ADC(cpu, *E);
			break;

		case 0x8c:
			ADC(cpu, *H);
			break;

		case 0x8d:
			ADC(cpu, *L);
			break;

		case 0x8e:
		{
			uint8_t value = cpu->memory[address];

			ADC(cpu, value);
			break;
		}

		case 0x8f:
			ADC(cpu, *A);
			break;

		// SUBs
		case 0x90:
			SUB(cpu, *B);
			break;

		case 0x91:
			SUB(cpu, *C);
			break;

		case 0x92:
			SUB(cpu, *D);
			break;

		case 0x93:
			SUB(cpu, *E);
			break;

		case 0x94:
			SUB(cpu, *H);
			break;

		case 0x95:
			SUB(cpu, *L);
			break;

		case 0x96:
		{
			uint8_t value = cpu->memory[address];

			SUB(cpu, value);
			break;
		}

		case 0x97:
			SUB(cpu, *A);
			break;

		// SBBs
		case 0x98:
			SBB(cpu, *B);
			break;

		case 0x99:
			SBB(cpu, *C);
			break;

		case 0x9a:
			SBB(cpu, *D);
			break;

		case 0x9b:
			SBB(cpu, *E);
			break;

		case 0x9c:
			SBB(cpu, *H);
			break;

		case 0x9d:
			SBB(cpu, *L);
			break;

		case 0x9e:
			{
				uint8_t value = cpu->memory[address];

				SBB(cpu, value);
				break;
			}

		case 0x9f:
			SBB(cpu, *A);
			break;

		// ANAs
		case 0xa0:
			ANA(cpu, *B);
			break;

		case 0xa1:
			ANA(cpu, *C);
			break;

		case 0xa2:
			ANA(cpu, *D);
			break;

		case 0xa3:
			ANA(cpu, *E);
			break;

		case 0xa4:
			ANA(cpu, *H);
			break;

		case 0xa5:
			ANA(cpu, *L);
			break;

		case 0xa6:
		{
			uint8_t value = cpu->memory[address];

			ANA(cpu, value);
			break;
		}

		case 0xA7:
			ANA(cpu, *A);
			break;

		// XRAs
		case 0xA8:
			XRA(cpu, *B);
			break;

		case 0xA9:
			XRA(cpu, *C);
			break;

		case 0xAA:
			XRA(cpu, *D);
			break;

		case 0xAB:
			XRA(cpu, *E);
			break;

		case 0xAC:
			XRA(cpu, *H);
			break;

		case 0xAD:
			XRA(cpu, *L);
			break;

		case 0xAE:
		{
			uint8_t value = cpu->memory[address];

			XRA(cpu, value);
			break;
		}

		case 0xAF:
			XRA(cpu, *A);
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
			uint8_t value = cpu->memory[address];

			ORA(cpu, &value);
			break;
		}

		case 0xB7:
			ORA(cpu, &cpu->registers.A);
			break;

		// CMPs
		case 0xB8:
			CMP(cpu, cpu->registers.B);
			break;

		case 0xB9:
			CMP(cpu, cpu->registers.C);
			break;

		case 0xBA:
			CMP(cpu, cpu->registers.D);
			break;

		case 0xBB:
			CMP(cpu, cpu->registers.E);
			break;

		case 0xBC:
			CMP(cpu, cpu->registers.H);
			break;

		case 0xBD:
			CMP(cpu, cpu->registers.L);
			break;

		case 0xBE:
		{
			uint8_t value = cpu->memory[address];

			CMP(cpu, value);
			break;
		}

		case 0xBF:
			CMP(cpu, cpu->registers.A);
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
			PUSH(cpu, cpu->registers.B, cpu->registers.C);
			break;
	   
		case 0xC6:
			ADI(cpu);
			break;
		
		case 0xC7:
			RST(cpu, 0);
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
			RST(cpu, 1);
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

		case 0xD5:
			PUSH(cpu, cpu->registers.D, cpu->registers.E);
			break;

		case 0xD6:
			SUI(cpu);
			break;

		case 0xD7:
			RST(cpu, 2);
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
			RST(cpu, 3);
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
			PUSH(cpu, cpu->registers.H, cpu->registers.L);
			break;
		
		case 0xE6:
			ANI(cpu);
			break;

		case 0xE7:
			RST(cpu, 4);
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
			RST(cpu, 5);
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
			RST(cpu, 6);
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
			RST(cpu, 7);
			break;

		default:
			printf("Unimplemented instruction: 0x%02X\n", instruction);
			error_occurred = 5;
			break;
	}

	uint8_t instruction_cycles = INSTRUCTION_CYCLES[instruction];

	for (int i = 0; i < instruction_cycles; i++)
	{
		cpu->cycles += 1;
		timer_irq(cpu);
		vblank_irq(cpu);
	}

	return instruction_cycles;
}

static inline void load_and_initialize(Cpu8080 *cpu) 
{
	load_rom(cpu);
	load_rom_to_memory(cpu);
	buffer_to_screen(cpu);
	update_screen();
}

static inline void handle_sdl_events(int *running) 
{
	SDL_Event event;
	while (SDL_PollEvent(&event)) {
		if (event.type == SDL_QUIT) {
			*running = 0;
			exit(0);
		}
	}
}

void intel8080_main(Cpu8080 *cpu)
{
	int running = 1; /* Flag to control loop execution */

	load_and_initialize(cpu);

	while (running)
	{
		handle_sdl_events(&running);

		uint8_t instruction_cycles = emulate_instruction(cpu);

		uint32_t instruction_time_ms = CYCLES_TO_MS(instruction_cycles);

		uint32_t instruction_time_seconds = instruction_time_ms * 1000;

		if (instruction_time_seconds > 0)
		{
			SDL_Delay(instruction_time_seconds);
		}
	}
}
