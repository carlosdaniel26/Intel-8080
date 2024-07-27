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
    // opcode table
    switch (*opcode)
    {
        case 0x00:
            printf("NOP\n");
            break;
        case 0x01:
            printf("LXI B, D16\n");
            break;
        case 0x02:
            printf("STAX B\n");
            break;
        case 0x03:
            printf("INX B\n");
            break;
        case 0x04:
            printf("INR B\n");
            break;
        case 0x05:
            printf("DCR B\n");
            break;
        case 0x06:
            printf("MVI B, D8\n");
            break;
        case 0x07:
            printf("RLC\n");
            break;
        case 0x08:
            printf("NOP\n");
            break;
        case 0x09:
            printf("DAD B\n");
            break;
        case 0x0A:
            printf("LDAX B\n");
            break;
        case 0x0B:
            printf("DCX B\n");
            break;
        case 0x0C:
            printf("INR C\n");
            break;
        case 0x0D:
            printf("DCR C\n");
            break;
        case 0x0E:
            printf("MVI C, D8\n");
            break;
        case 0x0F:
            printf("RRC\n");
            break;
        case 0x10:
            printf("NOP\n");
            break;
        case 0x11:
            printf("LXI D, D16\n");
            break;
        case 0x12:
            printf("STAX D\n");
            break;
        case 0x13:
            printf("INX D\n");
            break;
        case 0x14:
            printf("INR D\n");
            break;
        case 0x15:
            printf("DCR D\n");
            break;
        case 0x16:
            printf("MVI D, D8\n");
            break;
        case 0x17:
            printf("RAL\n");
            break;
        case 0x18:
            printf("NOP\n");
            break;
        case 0x19:
            printf("DAD D\n");
            break;
        case 0x1A:
            printf("LDAX D\n");
            break;
        case 0x1B:
            printf("DCX D\n");
            break;
        case 0x1C:
            printf("INR E\n");
            break;
        case 0x1D:
            printf("DCR E\n");
            break;
        case 0x1E:
            printf("MVI E, D8\n");
            break;
        case 0x1F:
            printf("RAR\n");
            break;
        case 0x20:
            printf("NOP\n");
            break;
        case 0x21:
            printf("LXI H, D16\n");
            break;
        case 0x22:
            printf("SHLD a16\n");
            break;
        case 0x23:
            printf("INX H\n");
            break;
        case 0x24:
            printf("INR H\n");
            break;
        case 0x25:
            printf("DCR H\n");
            break;
        case 0x26:
            printf("MVI H, D8\n");
            break;
        case 0x27:
            printf("DAA\n");
            break;
        case 0x28:
            printf("NOP\n");
            break;
        case 0x29:
            printf("DAD H\n");
            break;
        case 0x2A:
            printf("LHLD a16\n");
            break;
        case 0x2B:
            printf("DCX H\n");
            break;
        case 0x2C:
            printf("INR L\n");
            break;
        case 0x2D:
            printf("DCR L\n");
            break;
        case 0x2E:
            printf("MVI L, D8\n");
            break;
        case 0x2F:
            printf("CMA\n");
            break;
        case 0x30:
            printf("NOP\n");
            break;
        case 0x31:
            printf("LXI SP, D16\n");
            break;
        case 0x32:
            printf("STA a16\n");
            break;
        case 0x33:
            printf("INX SP\n");
            break;
        case 0x34:
            printf("INR M\n");
            break;
        case 0x35:
            printf("DCR M\n");
            break;
        case 0x36:
            printf("MVI M, D8\n");
            break;
        case 0x37:
            printf("STC\n");
            break;
        case 0x38:
            printf("NOP\n");
            break;
        case 0x39:
            printf("DAD SP\n");
            break;
        case 0x3A:
            printf("LDA a16\n");
            break;
        case 0x3B:
            printf("DCX SP\n");
            break;
        case 0x3C:
            printf("INR A\n");
            break;
        case 0x3D:
            printf("DCR A\n");
            break;
        case 0x3E:
            printf("MVI A, D8\n");
            break;
        case 0x3F:
            printf("CMC\n");
            break;
        case 0x40:
            printf("MOV B, B\n");
            break;
        case 0x41:
            printf("MOV B, C\n");
            break;
        case 0x42:
            printf("MOV B, D\n");
            break;
        case 0x43:
            printf("MOV B, E\n");
            break;
        case 0x44:
            printf("MOV B, H\n");
            break;
        case 0x45:
            printf("MOV B, L\n");
            break;
        case 0x46:
            printf("MOV B, M\n");
            break;
        case 0x47:
            printf("MOV B, A\n");
            break;
        case 0x48:
            printf("MOV C, B\n");
            break;
        case 0x49:
            printf("MOV C, C\n");
            break;
        case 0x4A:
            printf("MOV C, D\n");
            break;
        case 0x4B:
            printf("MOV C, E\n");
            break;
        case 0x4C:
            printf("MOV C, H\n");
            break;
        case 0x4D:
            printf("MOV C, L\n");
            break;
        case 0x4E:
            printf("MOV C, M\n");
            break;
        case 0x4F:
            printf("MOV C, A\n");
            break;
        case 0x50:
            printf("MOV D, B\n");
            break;
        case 0x51:
            printf("MOV D, C\n");
            break;
        case 0x52:
            printf("MOV D, D\n");
            break;
        case 0x53:
            printf("MOV D, E\n");
            break;
        case 0x54:
            printf("MOV D, H\n");
            break;
        case 0x55:
            printf("MOV D, L\n");
            break;
        case 0x56:
            printf("MOV D, M\n");
            break;
        case 0x57:
            printf("MOV D, A\n");
            break;
        case 0x58:
            printf("MOV E, B\n");
            break;
        case 0x59:
            printf("MOV E, C\n");
            break;
        case 0x5A:
            printf("MOV E, D\n");
            break;
        case 0x5B:
            printf("MOV E, E\n");
            break;
        case 0x5C:
            printf("MOV E, H\n");
            break;
        case 0x5D:
            printf("MOV E, L\n");
            break;
        case 0x5E:
            printf("MOV E, M\n");
            break;
        case 0x5F:
            printf("MOV E, A\n");
            break;
        case 0x60:
            printf("MOV H, B\n");
            break;
        case 0x61:
            printf("MOV H, C\n");
            break;
        case 0x62:
            printf("MOV H, D\n");
            break;
        case 0x63:
            printf("MOV H, E\n");
            break;
        case 0x64:
            printf("MOV H, H\n");
            break;
        case 0x65:
            printf("MOV H, L\n");
            break;
        case 0x66:
            printf("MOV H, M\n");
            break;
        case 0x67:
            printf("MOV H, A\n");
            break;
    }
}

