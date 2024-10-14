#ifndef DEBUG_H
#define DEBUG_H

#include "cpu.h"

void log_message(const char *format, ...);
void print_debug_message(const char *format, ...);
void update_clock_debug(Cpu8080* cpu);
void start_clock_debug(Cpu8080* cpu);
void log_8080(const char *message);
void print_opcode(const uint8_t *opcode);

#endif // DEBUG_H
