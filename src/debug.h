#ifndef DEBUG_H
#define DEBUG_H

#include "cpu.h"

void update_clock_debug(Cpu8080* cpu);
void start_clock_debug(Cpu8080* cpu);
void log_8080(const char *message);

#endif // DEBUG_H
