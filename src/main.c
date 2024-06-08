#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

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

} Cpu8080;   

char* rom;

void log_8080(const char *message)
{
    printf("8080 EMULATOR: %s", message);
}

// ==== ROM
char* get_rom()
{
    log_8080("loading rom...\n");

    char *source = NULL;
    FILE *fp = fopen(ROM_FILE, "r");
    if (fp != NULL) {
        /* Go to the end of the file. */
        if (fseek(fp, 0L, SEEK_END) == 0) {
            /* Get the size of the file. */
            long bufsize = ftell(fp);
            if (bufsize == -1) { /* Error */ }

            /* Allocate our buffer to that size. */
            source = malloc(sizeof(char) * (bufsize + 1));

            /* Go back to the start of the file. */
            if (fseek(fp, 0L, SEEK_SET) != 0) { /* Error */ }

            /* Read the entire file into memory. */
            size_t newLen = fread(source, sizeof(char), bufsize, fp);
            if ( ferror( fp ) != 0 ) {
                fputs("Error reading file", stderr);
            } else {
                source[newLen++] = '\0'; /* Just to be safe. */
            }
        }
        fclose(fp);
    }
    
    return source;

}

int get_rom_size()
{
    log_8080("loading rom size...\n");

    FILE *fp = fopen(ROM_FILE, "r");
    if (fp != NULL) {

        /* Go to the end of the file. */
        if (fseek(fp, 0L, SEEK_END) == 0) {
            /* Get the size of the file. */
            int bufsize = ftell(fp);
            printf("tamanho da rom: %d\n", bufsize);
            
            return bufsize;
        }
        fclose(fp);
    }
    
    return 0;

}

// EMULATOR STUFF
void init_cpu(Cpu8080 *cpu)
{
    cpu->registers.A = 0;
    cpu->registers.B = 0;
    cpu->registers.C = 0;
    cpu->registers.D = 0;
    cpu->registers.E = 0;
    cpu->registers.H = 0;
    cpu->registers.L = 0;
    cpu->registers.sp = 0;
    cpu->registers.pc = 0;
    
    cpu->memory = (uint8_t*)malloc(0x10000);  // 64KB 
    printf("cpu initialized\n");
}

// instructions

void NOP(Cpu8080 *cpu)
{
}

void JUMP(Cpu8080 *cpu)
{
    int pc = cpu->registers.pc;

    cpu->registers.pc = (rom[pc + 2] << 8) | rom[pc + 1];

    printf("jump to %u\n", cpu->registers.pc);

    printf("1: %d, 2: %d\n", rom[pc + 1], rom[pc + 2]);

    cpu->registers.pc += 2;
}


// main emulator function
void emulate(Cpu8080 *cpu)
{
    rom            = get_rom();
    int   rom_size = get_rom_size();

    if (rom == NULL) {
        fprintf(stderr, "Failed to load ROM\n");
        return;
    }

    init_cpu(cpu);

    uint8_t   instruction = 0;

    log_8080("emule started\n");

    while(cpu->registers.pc < rom_size)
    {
        unsigned int pc = cpu->registers.pc;

        instruction = rom[pc];
        
        printf("instruction = %d\n", instruction);

        if (instruction == 0x00)
        {
            NOP(cpu);
        }

        else if (instruction == 0xC3)
        {
            JUMP(cpu);
        }

        if (pc > 65492 + 10)
            exit(1);
        
        cpu->registers.pc++;
    }
    printf("emulaçao acabou\n");
}

int main ()
{
    Cpu8080 *cpu = (Cpu8080*)calloc(1, sizeof(Cpu8080));

    emulate(cpu);
    return 0;
}
