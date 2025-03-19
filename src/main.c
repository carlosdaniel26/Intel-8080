#include <stdio.h>
#include <SDL2/SDL.h>

#include <main.h>
#include <cpu.h>
#include <screen.h>

int main()
{
    init_screen();
    Cpu8080 *cpu =  init_cpu();

    intel8080_main(cpu);
    
    finish_and_free(cpu);

    return 0;
}
