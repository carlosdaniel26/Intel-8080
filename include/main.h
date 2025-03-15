#ifndef MAIN_H
#define MAIN_H

#include <stdio.h>
#include "cpu.h"

#define WIDTH  256
#define HEIGHT 224

static inline void init_sdl();
static inline void create_window();
static inline void create_render();
static inline void create_texture();
static inline void init_sdl_screen_buffer();
void update_screen();
static inline void finish_and_free();
void video_buffer_to_screen(Cpu8080 *cpu);

#endif