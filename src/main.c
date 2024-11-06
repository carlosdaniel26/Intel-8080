#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>
#include <errno.h>
#include <SDL2/SDL.h>

#include "main.h"
#include "helper.h"
#include "cpu.h"
#include "debug.h"
#include "rom.h"
#include "test.h"

SDL_Window *window;
SDL_Renderer *renderer;
SDL_Texture *texture;
Uint32 screen_buffer[VIDEO_RAM_SIZE * 8];

void init_sdl()
{
    if (SDL_Init(SDL_INIT_VIDEO) < 0)
    {
        fprintf(stderr, "Não foi possível inicializar SDL: %s\n", SDL_GetError());
        exit(1);
    }

    create_window();
    create_render();
    create_texture();
    init_sdl_screen_buffer();
}

void create_window()
{
    window = SDL_CreateWindow(
        "Intel 8080",
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        WIDTH, HEIGHT,
        SDL_WINDOW_SHOWN
    );

    if (! window)
    {
        fprintf(stderr, "Não foi possível criar a janela: %s\n", SDL_GetError());
        SDL_Quit();
		exit(1);
    }

}

void create_render()
{
    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

    if (! renderer)
    {
        fprintf(stderr, "An error ocurred while trying to create the renderer: %s\n", SDL_GetError());
        SDL_DestroyWindow(window);
        SDL_Quit();
        exit(1);
    }

}

void create_texture()
{
    /* Cria uma textura para o buffer de vídeo */
    texture = SDL_CreateTexture(
        renderer,
        SDL_PIXELFORMAT_RGBA8888,
        SDL_TEXTUREACCESS_STREAMING,
        WIDTH,
        HEIGHT
    );

    if (! texture)
    {
        fprintf(stderr, "An error ocurred while trying to create the texture: %s\n", SDL_GetError());
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        SDL_Quit();
        exit(1);
    }	
}

void init_sdl_screen_buffer()
{
    SDL_PixelFormat *format = SDL_AllocFormat(SDL_PIXELFORMAT_RGBA8888);

    /* Initialize in black screen */
    for (unsigned index = 0; index < (WIDTH * HEIGHT); index++)
    {
        screen_buffer[index] = SDL_MapRGBA(format, 255, 0, 0, 255); // Red if the videobuffer be filled worng
    }
}

void update_screen()
{
	/* Update the texture with videobuffer */
    SDL_UpdateTexture(texture, NULL, screen_buffer, (WIDTH) * sizeof(Uint32));
    /* Clean screen(renderer) */
    SDL_RenderClear(renderer);
    /* Draw the texture on the renderer */
    SDL_RenderCopy(renderer, texture, NULL, NULL);
    /* Update screen with new renderer */ 
	SDL_RenderPresent(renderer);

    /* Delay for the cpu */
    //SDL_Delay(16);  /* Aprox 60 FPS */

}

void finish_and_free(Cpu8080 *cpu)
{
    /* Free memory and close SDL */
    SDL_DestroyTexture(texture);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

	free(cpu->rom);
    free(cpu->memory);

}

void video_buffer_to_screen(Cpu8080 *cpu) 
{
    for (int i = VIDEO_RAM_START; i < (VIDEO_RAM_END-1); i++) {
        for (int bit = 0; bit < 8; bit++) {
            uint8_t bit_choosed = (cpu->memory[i] >> (7 - bit)) & 1;

            Uint8 r, g, b, a = 255;

            if (bit_choosed == 0) {
                r = 0;
                g = 0;
                b = 0;
            } else {
                r = 255;
                g = 255;
                b = 255;
            }

            unsigned index = (((i - VIDEO_RAM_START) * 8) + bit);

            screen_buffer[index] = SDL_MapRGBA(SDL_AllocFormat(SDL_PIXELFORMAT_RGBA8888), r, g, b, a);
        }
    }
}

int main()
{
    #ifdef TEST
    test_main();
    #endif

    init_sdl();
    Cpu8080 *cpu =  init_cpu();

	intel8080_main(cpu);
    
	finish_and_free(cpu);

    return 0;
}
