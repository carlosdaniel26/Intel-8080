#ifndef SCREEN_H
#define SCREEN_H

#include <cpu.h>

void create_window();
void create_render();
void create_texture();
void init_sdl_screen_buffer();
void update_screen();
void finish_and_free(Cpu8080 *cpu);
void video_buffer_to_screen(Cpu8080 *cpu);
void init_screen();

#endif