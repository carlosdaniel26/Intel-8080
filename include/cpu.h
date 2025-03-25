#include <stdint.h>
#include <stdbool.h>

#include <helper.h>

#ifndef CPU_H
#define CPU_H

#define TARGET_FPS 60

#define MHz_TO_HZ(mhz) ((mhz) * 1000000)
#define CYCLES_PER_MS(cycles_per_second) ((cycles_per_second) / 1000)

#define CPU_CLOCK 2
#define CYCLES_PER_SECOND MHz_TO_HZ(CPU_CLOCK)
#define CYCLES_PER_FRAME (CYCLES_PER_SECOND / TARGET_FPS)

#define CYCLES_TO_MS(cycles) ((cycles) / CYCLES_PER_MS(CYCLES_PER_SECOND))

#define TOTAL_MEMORY_SIZE 0x10000  // 64 KB

#define VIDEO_RAM_START 0x2400
#define VIDEO_RAM_END   0x3FFF
#define VIDEO_RAM_SIZE  ((VIDEO_RAM_END - VIDEO_RAM_START)+1)

#define P1_PORT      0x00
#define P2_PORT      0x01

#define STATUS_PORT  0x03
#define JOYSTICK_PORT 0x04
#define SOUND_PORT   0x02
#define VIDEO_PORT   0x05
#define LED_PORT     0x06
#define TIMER_PORT   0x07

#define TIMER_INTERRUPT_CYCLES (CYCLES_PER_SECOND / 120)
#define VBLANK_INTERRUPT_CYCLES (CYCLES_PER_SECOND / 120)
#define INPUT_POLLING_CYCLES (CYCLES_PER_SECOND / 60)
#define SOUND_INTERRUPT_CYCLES (CYCLES_PER_SECOND / 60)

#define ISRDELAY 0x20C0


typedef struct Flags {
	uint8_t		z:1;
	uint8_t		s:1;
	uint8_t		p:1;
	uint8_t		cy:1;
	uint8_t		ac:1;
	uint8_t		pad:3;
} Flags;

typedef struct Registers {
    uint8_t A;    
    uint8_t B;    
    uint8_t C;    
    uint8_t D;    
    uint8_t E;
    Flags   F;
    uint8_t H;    
    uint8_t L;

    // SPECIAL
    uint16_t sp;
    unsigned int pc;

} Registers;

typedef struct Cpu8080 {    
    Registers registers;
    uint8_t *memory;   
    char *rom;
	bool interrupt_enabled;
    uint64_t cycles;
} Cpu8080;

Cpu8080* init_cpu();
void intel8080_main(Cpu8080 *cpu);

// Add after the CPU_CLOCK define

static const uint8_t INSTRUCTION_CYCLES[256] = {

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
    [0x3F] = 4,  // CMC

    [0x40] = 4, // MOV B, B
    [0x41] = 4, // MOV B, C
    [0x42] = 4, // MOV B, D
    [0x43] = 4, // MOV B, E
    [0x44] = 4, // MOV B, H
    [0x45] = 4, // MOV B, L
    [0x46] = 7, // MOV B, M
    [0x47] = 4, // MOV B, A

    [0x48] = 4, // MOV C, B
    [0x49] = 4, // MOV C, C
    [0x4A] = 4, // MOV C, D
    [0x4B] = 4, // MOV C, E
    [0x4C] = 4, // MOV C, H
    [0x4D] = 4, // MOV C, L
    [0x4E] = 7, // MOV C, M
    [0x4F] = 4, // MOV C, A

    [0x50] = 4, // MOV D, B
    [0x51] = 4, // MOV D, C
    [0x52] = 4, // MOV D, D
    [0x53] = 4, // MOV D, E
    [0x54] = 4, // MOV D, H
    [0x55] = 4, // MOV D, L
    [0x56] = 7, // MOV D, M
    [0x57] = 4, // MOV D, A

    [0x58] = 4, // MOV E, B
    [0x59] = 4, // MOV E, C
    [0x5A] = 4, // MOV E, D
    [0x5B] = 4, // MOV E, E
    [0x5C] = 4, // MOV E, H
    [0x5D] = 4, // MOV E, L
    [0x5E] = 7, // MOV E, M
    [0x5F] = 4, // MOV E, A

    [0x60] = 4, // MOV H, B
    [0x61] = 4, // MOV H, C
    [0x62] = 4, // MOV H, D
    [0x63] = 4, // MOV H, E
    [0x64] = 4, // MOV H, H
    [0x65] = 4, // MOV H, L
    [0x66] = 7, // MOV H, M
    [0x67] = 4, // MOV H, A

    [0x68] = 4, // MOV L, B
    [0x69] = 4, // MOV L, C
    [0x6A] = 4, // MOV L, D
    [0x6B] = 4, // MOV L, E
    [0x6C] = 4, // MOV L, H
    [0x6D] = 4, // MOV L, L
    [0x6E] = 7, // MOV L, M
    [0x6F] = 4, // MOV L, A

    [0x70] = 7, // MOV M, B
    [0x71] = 7, // MOV M, C
    [0x72] = 7, // MOV M, D
    [0x73] = 7, // MOV M, E
    [0x74] = 7, // MOV M, H
    [0x75] = 7, // MOV M, L
    [0x76] = 4, // HLT
    [0x77] = 7, // MOV M, A

    [0x78] = 4, // MOV A, B
    [0x79] = 4, // MOV A, C
    [0x7A] = 4, // MOV A, D
    [0x7B] = 4, // MOV A, E
    [0x7C] = 4, // MOV A, H
    [0x7D] = 4, // MOV A, L
    [0x7E] = 7, // MOV A, M
    [0x7F] = 4, // MOV A, A

    [0x80] = 4, // ADD B
    [0x81] = 4, // ADD C
    [0x82] = 4, // ADD D
    [0x83] = 4, // ADD E
    [0x84] = 4, // ADD H
    [0x85] = 4, // ADD L
    [0x86] = 7, // ADD M
    [0x87] = 4, // ADD A

    [0x88] = 4, // ADC B
    [0x89] = 4, // ADC C
    [0x8A] = 4, // ADC D
    [0x8B] = 4, // ADC E
    [0x8C] = 4, // ADC H
    [0x8D] = 4, // ADC L
    [0x8E] = 7, // ADC M
    [0x8F] = 4, // ADC A

    [0x90] = 4, // SUB B
    [0x91] = 4, // SUB C
    [0x92] = 4, // SUB D
    [0x93] = 4, // SUB E
    [0x94] = 4, // SUB H
    [0x95] = 4, // SUB L
    [0x96] = 7, // SUB M
    [0x97] = 4, // SUB A

    [0x98] = 4, // SBB B
    [0x99] = 4, // SBB C
    [0x9A] = 4, // SBB D
    [0x9B] = 4, // SBB E
    [0x9C] = 4, // SBB H
    [0x9D] = 4, // SBB L
    [0x9E] = 7, // SBB M
    [0x9F] = 4, // SBB A

    [0xA0] = 4, // ANA B
    [0xA1] = 4, // ANA C
    [0xA2] = 4, // ANA D
    [0xA3] = 4, // ANA E
    [0xA4] = 4, // ANA H
    [0xA5] = 4, // ANA L
    [0xA6] = 7, // ANA M
    [0xA7] = 4, // ANA A

    [0xA8] = 4, // XRA B
    [0xA9] = 4, // XRA C
    [0xAA] = 4, // XRA D
    [0xAB] = 4, // XRA E
    [0xAC] = 4, // XRA H
    [0xAD] = 4, // XRA L
    [0xAE] = 7, // XRA M
    [0xAF] = 4, // XRA A

    [0xB0] = 4, // ORA B
    [0xB1] = 4, // ORA C
    [0xB2] = 4, // ORA D
    [0xB3] = 4, // ORA E
    [0xB4] = 4, // ORA H
    [0xB5] = 4, // ORA L
    [0xB6] = 7, // ORA M
    [0xB7] = 4, // ORA A

    [0xB8] = 4, // RIM
    [0xB9] = 4, // SIM
    [0xBA] = 4, // - (Not used/NOP)
    [0xBB] = 4, // - (Not used/NOP)
    [0xBC] = 4, // - (Not used/NOP)
    [0xBD] = 4, // - (Not used/NOP)
    [0xBE] = 4, // - (Not used/NOP)
    [0xBF] = 4, // - (Not used/NOP)

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

#endif
