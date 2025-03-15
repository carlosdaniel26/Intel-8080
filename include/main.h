#ifndef MAIN_H
#define MAIN_H

#include <stdio.h>
#include "cpu.h"

#define WIDTH  256
#define HEIGHT 224

void update_screen(void);
void video_buffer_to_screen(Cpu8080 *cpu);


#endif