#include <stdint.h>
#include <stdbool.h>

#ifndef CPU_H
#define CPU_H

#define TARGET_FPS 60

#define CPU_CLOCK 2 // MHz
#define CYCLES_PER_SECOND (CPU_CLOCK * 1000000)
#define CYCLES_PER_FRAME (CYCLES_PER_SECOND / TARGET_FPS)

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

#define TIMER_INTERRUPT_CYCLES (CYCLES_PER_SECOND / 60)
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
    uint64_t cycle_count;
} Cpu8080;

Cpu8080* init_cpu();
void intel8080_main(Cpu8080 *cpu);

// Add after the CPU_CLOCK define

static const uint8_t INSTRUCTION_CYCLES[256] = {
    4,  10, 7,  5,  5,  5,  7,  4,  4,  10, 7,  5,  5,  5,  7,  4,  //0x00-0x0f
    4,  10, 7,  5,  5,  5,  7,  4,  4,  10, 7,  5,  5,  5,  7,  4,  //0x10-0x1f
    4,  10, 16, 5,  5,  5,  7,  4,  4,  10, 16, 5,  5,  5,  7,  4,  //0x20-0x2f
    4,  10, 13, 5,  10, 10, 10, 4,  4,  10, 13, 5,  5,  5,  7,  4,  //0x30-0x3f
    5,  5,  5,  5,  5,  5,  7,  5,  5,  5,  5,  5,  5,  5,  7,  5,  //0x40-0x4f
    5,  5,  5,  5,  5,  5,  7,  5,  5,  5,  5,  5,  5,  5,  7,  5,  //0x50-0x5f
    5,  5,  5,  5,  5,  5,  7,  5,  5,  5,  5,  5,  5,  5,  7,  5,  //0x60-0x6f
    7,  7,  7,  7,  7,  7,  7,  7,  5,  5,  5,  5,  5,  5,  7,  5,  //0x70-0x7f
    4,  4,  4,  4,  4,  4,  7,  4,  4,  4,  4,  4,  4,  4,  7,  4,  //0x80-0x8f
    4,  4,  4,  4,  4,  4,  7,  4,  4,  4,  4,  4,  4,  4,  7,  4,  //0x90-0x9f
    4,  4,  4,  4,  4,  4,  7,  4,  4,  4,  4,  4,  4,  4,  7,  4,  //0xa0-0xaf
    4,  4,  4,  4,  4,  4,  7,  4,  4,  4,  4,  4,  4,  4,  7,  4,  //0xb0-0xbf
    11, 10, 10, 10, 17, 11, 7,  11, 11, 10, 10, 10, 17, 17, 7,  11, //0xc0-0xcf
    11, 10, 10, 10, 17, 11, 7,  11, 11, 10, 10, 10, 17, 17, 7,  11, //0xd0-0xdf
    11, 10, 10, 18, 17, 11, 7,  11, 11, 5,  10, 4,  17, 17, 7,  11, //0xe0-0xef
    11, 10, 10, 4,  17, 11, 7,  11, 11, 5,  10, 4,  17, 17, 7,  11  //0xf0-0xff
};

#endif
