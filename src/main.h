#include <stdint.h>

#define clear() printf("\033[H\033[J")

#define ROM_FILE "./rom/invaders"


typedef struct Registers {
    uint8_t      A;    
    uint8_t      B;    
    uint8_t      C;    
    uint8_t      D;    
    uint8_t      E;    
    uint8_t      H;    
    uint8_t      L;   

    // SPECIAL
    uint16_t             sp;    
    unsigned int         pc;    

} Registers;

typedef struct Cpu8080 {    

    Registers registers;
    
    uint8_t    *memory;   
    char       *rom; 

} Cpu8080;

void start_clock_debug(Cpu8080* cpu);
void update_clock_debug();
