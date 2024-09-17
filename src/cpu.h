#include <stdint.h>
#include <stdbool.h>

#ifndef CPU_H
#define CPU_H

typedef struct Registers {
    uint8_t A;    
    uint8_t B;    
    uint8_t C;    
    uint8_t D;    
    uint8_t E;
    uint8_t F;
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

#endif
