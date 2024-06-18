#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

#include "cpu.h"
#include "debug.h"
#include "rom.h"

// CPU initialization
void init_cpu(Cpu8080 *cpu) {
    cpu->registers = (Registers){0}; // Initialize all registers to 0
    cpu->memory = (uint8_t*)calloc(0x10000, sizeof(uint8_t)); // 64KB
    if (!cpu->memory) {
        perror("Memory allocation error");
        exit(EXIT_FAILURE);
    }
    printf("cpu initialized\n");
}

// Instructions
void NOP() {
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

void MOV_B_C(Cpu8080 *cpu) {
    cpu->registers.B = cpu->registers.C;
}

void MOV_B_D(Cpu8080 *cpu) {
    cpu->registers.B = cpu->registers.D;
}

void MOV_B_E(Cpu8080 *cpu) {
    cpu->registers.B = cpu->registers.E;
}

void ADD_B(Cpu8080 *cpu) {
    cpu->registers.A += cpu->registers.B;
}

// Main emulator function
void emulate(Cpu8080 *cpu) {
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
                NOP();
                break;
            case 0xC3:
                JUMP(cpu);
                break;
            case 0x01:
                LXI(cpu, &cpu->registers.B, &cpu->registers.C);
                break;
            case 0x41:
                MOV_B_C(cpu);
                break;
            case 0x42:
                MOV_B_D(cpu);
                break;
            case 0x43:
                MOV_B_E(cpu);
                break;
            case 0x80:
                ADD_B(cpu);
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
