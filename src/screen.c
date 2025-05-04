#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <SDL2/SDL.h>

#include <main.h>
#include <helper.h>
#include <cpu.h>

#define SCREEN_PROPORTION 2

SDL_Window *window;
SDL_Renderer *renderer;
SDL_Texture *texture;
SDL_PixelFormat *format;
Uint32 screen_buffer[VIDEO_RAM_SIZE * 8];

void create_window()
{
    window = SDL_CreateWindow(
        "Intel 8080",
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        HEIGHT * SCREEN_PROPORTION, WIDTH * SCREEN_PROPORTION,
        SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE);

    if (!window)
    {
        fprintf(stderr, "Failed to create window: %s\n", SDL_GetError());
        SDL_Quit();
        exit(1);
    }
}

void create_render()
{
    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

    if (!renderer)
    {
        fprintf(stderr, "Failed to create renderer: %s\n", SDL_GetError());
        SDL_DestroyWindow(window);
        SDL_Quit();
        exit(1);
    }
}

void create_texture()
{
    texture = SDL_CreateTexture(
        renderer,
        SDL_PIXELFORMAT_RGBA8888,
        SDL_TEXTUREACCESS_STREAMING,
        WIDTH,
        HEIGHT);

    if (!texture)
    {
        fprintf(stderr, "Failed to create texture: %s\n", SDL_GetError());
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        SDL_Quit();
        exit(1);
    }
}

void init_sdl_screen_buffer()
{
    for (unsigned index = 0; index < (WIDTH * HEIGHT); index++)
    {
        screen_buffer[index] = SDL_MapRGBA(format, 255, 0, 0, 255); // Red for debugging
    }
}

inline static SDL_Rect calculate_dest_rect(SDL_Window *window, int texture_width, int texture_height)
{
    int window_width, window_height;
    SDL_GetWindowSize(window, &window_width, &window_height);

    /* Calculate the destination rectangle to fit the texture proportionally */

    float aspect_ratio = (float)texture_width / texture_height;
    int dest_width, dest_height;

    if ((float)window_width / window_height > aspect_ratio)
    {
        dest_height = window_height;
        dest_width = (int)(window_height * aspect_ratio);
    }
    else
    {
        dest_width = window_width;
        dest_height = (int)(window_width / aspect_ratio);
    }

    int centerX = (window_width - dest_width) / 2;
    int centerY = (window_height - dest_height) / 2;

    return (SDL_Rect){centerX, centerY, dest_width, dest_height};
}

void update_screen()
{
    if (texture == NULL) return;
    
    SDL_UpdateTexture(texture, NULL, screen_buffer, WIDTH * sizeof(Uint32));
    SDL_RenderClear(renderer);

    SDL_Rect dest_rect = calculate_dest_rect(window, WIDTH, HEIGHT);

    /* Draw with a 90-degree rotation */
    SDL_RenderCopyEx(renderer, texture, NULL, &dest_rect, 90, NULL, SDL_FLIP_NONE);

    SDL_RenderPresent(renderer);
}

void finish_and_free(Cpu8080 *cpu)
{
    if (format)
        SDL_FreeFormat(format);

    SDL_DestroyTexture(texture);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    if (cpu)
    {
        free(cpu->rom);
        free(cpu->memory);
    }
}

void buffer_to_screen(Cpu8080 *cpu)
{
    if (texture == NULL) return;

    uint8_t *buffer = (cpu->memory + VIDEO_RAM_START);

    for (unsigned byte = 0; byte < VIDEO_RAM_SIZE; byte++)
    {
        for (unsigned bit = 0; bit < 8; bit++)
        {
            uint8_t bit_choosed = (buffer[byte] >> (7 - bit)) & 1;

            unsigned index = ((VIDEO_RAM_SIZE - 1 - byte) * 8 + bit);

            Uint8 color = bit_choosed ? 255 : 0; /* White or Black*/

            screen_buffer[index] = SDL_MapRGBA(format, color, color, color, 255);
        }
    }
}

void init_screen()
{
    if (SDL_Init(SDL_INIT_VIDEO) < 0)
    {
        fprintf(stderr, "Failed to initialize SDL: %s\n", SDL_GetError());
        exit(1);
    }

    format = SDL_AllocFormat(SDL_PIXELFORMAT_RGBA8888);
    if (!format)
    {
        fprintf(stderr, "Failed to allocate pixel format: %s\n", SDL_GetError());
        exit(1);
    }

    create_window();
    create_render();
    create_texture();
    init_sdl_screen_buffer();
    update_screen();
}