#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

#include "cpu.h"
#include "debug.h"
#include "rom.h"

// CPU initialization
void init_cpu(Cpu8080 *cpu) 
{
    cpu->registers = (Registers){0}; // Initialize all registers to 0
    cpu->memory = (uint8_t*)calloc(0x10000, sizeof(uint8_t)); // 64KB
    if (!cpu->memory) {
        perror("Memory allocation error");
        exit(EXIT_FAILURE);
    }
    printf("cpu initialized\n");
}

// Instructions
void NOP() 
{
    return;
}

void JUMP(Cpu8080 *cpu) {
    uint8_t low = cpu->rom[cpu->registers.pc + 1];
    uint8_t high = cpu->rom[cpu->registers.pc + 2];
    cpu->registers.pc = (high << 8) | low;
}

void LXI(Cpu8080 *cpu, uint8_t *reg_high, uint8_t *reg_low) {
    *reg_low = cpu->rom[cpu->registers.pc + 1];
    *reg_high = cpu->rom[cpu->registers.pc + 2];
    cpu->registers.pc += 2;
}

void MOV_reg_to_reg(Cpu8080 *cpu, uint8_t *target, uint8_t *source)
{
    target = source;
}

void MOV_im_to_reg(Cpu8080 *cpu, uint8_t *target, uint8_t value)
{
    *target = value;
}

void ADD(Cpu8080 *cpu, uint8_t *_register)
{
    cpu->registers.A += _register;
}

void ADI(Cpu8080 *cpu, uint8_t value)
{
    cpu->registers.A += value;
}

void ADC(Cpu8080 *cpu, uint8_t *_register)
{
    uint16_t temp = value + cpu->registers.A + cpu->registers.C;

    cpu->registers.C = (temp > 0xFF);
    cpu->registers.A = temp & 0xFF
}


// Main emulator function
void emulate(Cpu8080 *cpu) {
    // foda-se
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
    // endfoda-se
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
        unsigned int pc = cpu->registers.pc;
        uint8_t instruction = cpu->rom[pc];

        switch (instruction) {
            case 0x00:
            case 0x08:
            case 0x10:
            case 0x11:
            case 0x20:
            case 0x21:
            case 0x30:
                NOP();
                break;

            case 0xC3:
                JUMP(cpu);
                break;

            case 0x01:
                LXI(cpu, &cpu->registers.B, &cpu->registers.C);
                break;

            case 0x41: // MOV B, D
                MOV_reg_to_reg(cpu, B, C);
                break;

            case 0x42: // MOV B, D
                MOV_reg_to_reg(cpu, B, D);
                break;

            case 0x43: // MOV B E
                MOV_reg_to_reg(cpu, B, E);
                break;
                
            case 0x80: // ADD B
                ADD(cpu, B);
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

int main() {
    Cpu8080 cpu;
    emulate(&cpu);
    return 0;
}
