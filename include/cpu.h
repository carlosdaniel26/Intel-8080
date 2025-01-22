#include <stdint.h>
#include <stdbool.h>

#ifndef CPU_H
#define CPU_H

#define CPU_CLOCK 2000000 // 2 MHz
#define CYCLES_TO_MS(cycles) ((cycles) * 1000 / CPU_CLOCK)

//#define TOTAL_MEMORY_SIZE 0x4000  // 16 KB
#define TOTAL_MEMORY_SIZE 0x10000  // 64 KB

#define VIDEO_RAM_START 0x2400
#define VIDEO_RAM_END   0x3FFF
#define VIDEO_RAM_SIZE  ((VIDEO_RAM_END - VIDEO_RAM_START)+1)

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
} Cpu8080;

Cpu8080* init_cpu();
void load_rom(Cpu8080 *cpu);
void intel8080_main(Cpu8080 *cpu);
void load_rom_to_memory(Cpu8080 *cpu);

void ADD(Cpu8080 *cpu, uint8_t byte);

#endif
