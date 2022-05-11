#ifndef WINDOW_H
#define WINDOW_H

#include <iostream>
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include "timer.h"

class Window
{
public:
    bool running = false;
    int windowWidth;
    int windowHeight;

private:
    /* Window */
    bool isFullscreen = false;
    SDL_Window *window;
    SDL_Renderer *renderer;

    /* Color buffer  */
    uint32_t *colorBuffer;
    SDL_Texture *colorBufferTexture;

    /* Text */
    SDL_Surface *textSurface;
    SDL_Color textColor = {255, 255, 255};
    TTF_Font *textFont;

    /* Fps */
    bool enableCap = true;
    int fpsCap = 60;
    float avgFPS = 0;
    int screenTicksPerFrame = 1000 / fpsCap;
    long countedFrames = 0;

    /* Timers */
    Timer fpsTimer, capTimer;

public:
    Window(int w, int h) : windowWidth(w), windowHeight(h){};
    ~Window();

    void Init();
    void Setup();

    void ProcessInput();
    void Update();
    void Render();
    void PostRender();

    void ClearColorBuffer(uint32_t color);
    void RenderColorBuffer();

    void DrawGrid(unsigned int color);
    void DrawPixel(int sx, int sy, unsigned int color);
    void DrawRect(int sx, int sy, int width, int height, uint32_t color);
};

#endif