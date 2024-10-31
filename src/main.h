#ifndef MAIN_H
#define MAIN_H

#include <stdio.h>
#include "cpu.h"

#define WIDTH  256
#define HEIGHT 224

void init_sdl();
void create_window();
void create_render();
void create_texture();
void init_sdl_screen_buffer();
void update_screen();
void finish_and_free();
void video_buffer_to_screen(Cpu8080 *cpu);

#endif