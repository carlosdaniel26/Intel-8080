#ifndef DEBUG_H
#define DEBUG_H

#include <stddef.h>

#include "cpu.h"

#define DEBUG_ON 0

void print_flag_register(Cpu8080 *cpu);
#endif // DEBUG_H
