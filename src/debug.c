#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <stdarg.h>
#include <time.h>
#include <sys/stat.h>

#include "debug.h"
#include "rom.h"
#include "main.h"
#include "helper.h"

#define clear() system("clear");

void print_flag_register(Cpu8080 *cpu) 
{
    uint8_t flag_register = cpu->registers.F;

    printf("Flag Register: 0x%02X\n", flag_register);
    printf("Sign (S): %s\n", (flag_register & FLAG_SIGN) ? "Set" : "Clear");
    printf("Zero (Z): %s\n", (flag_register & FLAG_ZERO) ? "Set" : "Clear");
    printf("Auxiliary Carry (AC): %s\n", (flag_register & FLAG_AUX_CARRY) ? "Set" : "Clear");
    printf("Parity (P): %s\n", (flag_register & FLAG_PARITY) ? "Set" : "Clear");
    printf("Carry (CY): %s\n", (flag_register & FLAG_CARRY) ? "Set" : "Clear");
}