#include <stdio.h>
#include <stdint.h>

#include "debug.h"

#define clear() printf("\033[H\033[J")

void update_clock_debug(Cpu8080* cpu) 
{
    clear();
    printf(" _______________ \n");
    printf("Current instruction:\n");
    printf("\tasm: "), print_opcode(&cpu->rom[cpu->registers.pc]);
    printf("\tinteger: %u\n", (uint8_t)cpu->rom[cpu->registers.pc]);
    printf("\thex: 0x%0x\n", (uint8_t)cpu->rom[cpu->registers.pc]);
    printf(" _______________ \n");
    printf("|               |\n");
    printf("|pc: %u          |\n", cpu->registers.pc);
    printf("|sp: %u          |\n", cpu->registers.sp);
    printf("|A:  %u          |\n", cpu->registers.A);
    printf("|B:  %u          |\n", cpu->registers.B);
    printf("|C:  %u          |\n", cpu->registers.C);
    printf("|D:  %u          |\n", cpu->registers.D);
    printf("|E:  %u          |\n", cpu->registers.E);
    printf("|H:  %u          |\n", cpu->registers.H);
    printf("|L:  %u          |\n", cpu->registers.L);
    printf("|               |\n");

    printf(" --------------- \n");
}

void start_clock_debug(Cpu8080* cpu) 
{
    clear();
    update_clock_debug(cpu);
}

void log_8080(const char *message) 
{
    printf("8080 EMULATOR: %s", message);
}

void print_opcode(const uint8_t *opcode)
{
    //opcode table
    switch (*opcode)
    {
        case 0x00:
        case 0x08:
        case 0x10:
        case 0x11:
        case 0x20:
        case 0x21:
        case 0x30:
            printf("NOP\n");
            break;

        default:
            printf("Instruction not implemented\n");
            break;
    }
}