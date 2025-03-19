#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>
#include <errno.h>
#include <SDL2/SDL.h>

#include <main.h>
#include <helper.h>
#include <cpu.h>
#include <debug.h>
#include <rom.h>
#include <screen.h>

int main()
{
    init_screen();
    Cpu8080 *cpu =  init_cpu();

    intel8080_main(cpu);
    
    finish_and_free(cpu);

    return 0;
}
