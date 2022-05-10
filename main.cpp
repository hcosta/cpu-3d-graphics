#include <iostream>
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include "timer.h"

bool is_running = false;
bool is_fullscreen = false;

SDL_Window *window;
SDL_Renderer *renderer;

uint32_t *color_buffer;
SDL_Texture *color_buffer_texture;

SDL_Surface *text;
SDL_Color color = {255, 255, 255};
TTF_Font *font;

int window_width = 640;
int window_height = 480;

float avgFPS = 0;

bool enableCap = true;
int fpsCap = 60;
int screenTicksPerFrame = 1000 / fpsCap;

bool initialize_window()
{
    // Inicializamos SDL
    if (SDL_Init(SDL_INIT_EVERYTHING) != 0)
    {
        std::cout << "Error initializing SDL." << std::endl;
        return false;
    }

    // Inicialización TTF
    if (TTF_Init() < 0)
    {
        std::cout << "Error initializing SDL_ttf: " << TTF_GetError() << std::endl;
        return false;
    }

    // Utilizar SDL para preguntar la resolucion maxima del monitor
    SDL_DisplayMode display_mode;
    SDL_GetCurrentDisplayMode(0, &display_mode);

    if (is_fullscreen)
    {
        window_width = display_mode.w;
        window_height = display_mode.h;
    }

    // Creamos la ventana SDL
    window = SDL_CreateWindow(
        NULL, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        window_width, window_height, SDL_WINDOW_BORDERLESS);

    if (!window)
    {
        std::cout << "Error creando la ventana SDL." << std::endl;
        return false;
    }

    // Creamos el renderizador SDL
    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (!renderer)
    {
        std::cout << "Error creando el renderizador SDL." << std::endl;
        return false;
    }

    // font setup
    font = TTF_OpenFont("assets/FreeSans.ttf", 16);
    if (!font)
    {
        std::cout << "Error loading font: " << TTF_GetError() << std::endl;
        return false;
    }

    if (is_fullscreen)
    {
        SDL_SetWindowFullscreen(window, SDL_WINDOW_FULLSCREEN);
    }

    return true;
}

void setup()
{
    // Reservar la memoria requerida en bytes para mantener el color buffer
    color_buffer = static_cast<uint32_t *>(malloc(sizeof(uint32_t) * window_width * window_height));

    // Crear la textura SDL utilizada para mostrar el color buffer
    color_buffer_texture = SDL_CreateTexture(
        renderer,
        SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING,
        window_width, window_height);
}

void process_input()
{
    SDL_Event event;
    SDL_PollEvent(&event);

    switch (event.type)
    {
    case SDL_QUIT:
        is_running = false;
        break;
    case SDL_KEYDOWN:
        if (event.key.keysym.sym == SDLK_ESCAPE)
            is_running = false;
        break;
    }
}

void update()
{
    // TODO:
}

void draw_grid(uint32_t color)
{
    for (size_t y = 0; y < window_height; y += 10)
    {
        for (size_t x = 0; x < window_width; x += 10)
        {
            color_buffer[(window_width * y) + x] = color;
        }
    }
}

void draw_rect(int sx, int sy, int width, int height, uint32_t color)
{
    for (size_t y = sy; (y < sy + height) && (y < window_height); y++)
    {
        for (size_t x = sx; (x < sx + width) && (x < window_width); x++)
        {
            color_buffer[(window_width * y) + x] = color;
        }
    }
}

void clear_color_buffer(uint32_t color)
{
    for (size_t y = 0; y < window_height; y++)
    {
        for (size_t x = 0; x < window_width; x++)
        {
            color_buffer[(window_width * y) + x] = color;
        }
    }
}

void render_color_buffer()
{
    // Copiar el color buffer y su contenido a la textura
    // Así podremos dibujar la textura en el renderer
    SDL_UpdateTexture(color_buffer_texture, NULL, color_buffer, window_width * sizeof(uint32_t));
    SDL_RenderCopy(renderer, color_buffer_texture, NULL, NULL);
}

void render()
{
    // Establecer el color del renderizador
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);

    // Limpiar la pantalla con el color establecido
    SDL_RenderClear(renderer);

    // Limpiar el color buffer
    clear_color_buffer(static_cast<uint32_t>(0xFF0000000));

    // Renderizar una cuadrícula
    draw_grid(static_cast<uint32_t>(0xFF616161));

    // Renderizar unos rectángulos
    draw_rect(50, 50, 100, 100, static_cast<uint32_t>(0xFF1570E8));
    draw_rect(205, 125, 300, 200, static_cast<uint32_t>(0xFFD93E23));
    draw_rect(375, 225, 300, 300, static_cast<uint32_t>(0xFFE35FDA));

    // Renderizar el color buffer
    render_color_buffer();

    // Render FPS
    text = TTF_RenderText_Solid(font, (std::to_string(avgFPS) + " fps").c_str(), color);
    if (!text)
    {
        std::cout << "Failed to render text: " << TTF_GetError() << std::endl;
    }
    SDL_Texture *text_texture = SDL_CreateTextureFromSurface(renderer, text);
    SDL_Rect dest = {2, 459, text->w, text->h};
    SDL_RenderCopy(renderer, text_texture, NULL, &dest);

    // Finalmente actualizar la pantalla
    SDL_RenderPresent(renderer);

    // Liberación de memoria local
    SDL_DestroyTexture(text_texture);
}

void destroy_window()
{
    // Liberar la memoria dinámica
    free(color_buffer);

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);

    SDL_FreeSurface(text);
    TTF_CloseFont(font);
    TTF_Quit();

    SDL_Quit();
}

int main(int argc, char *argv[])
{

    is_running = initialize_window();

    setup();

    // The frames per second timer
    Timer fpsTimer;

    // The frames per second cap timer
    Timer capTimer;

    // Start counting frames per second
    int countedFrames = 0;

    // Start fpsTimer
    fpsTimer.start();

    while (is_running)
    {
        // Start cap timer
        capTimer.start();
        process_input();

        update();

        // Calculate fps
        avgFPS = countedFrames / (fpsTimer.getTicks() / 1000.f);
        // Increment the frame counter
        ++countedFrames;

        render();

        // Si el fotograma finaliza demasiado pronto
        int frameTicks = capTimer.getTicks();
        if (enableCap && frameTicks < screenTicksPerFrame)
        {
            // Esperamos el tiempo restante
            SDL_Delay(screenTicksPerFrame - frameTicks);
        }
    }

    destroy_window();

    return 0;
}