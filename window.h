#ifndef WINDOW_H
#define WINDOW_H

#include <iostream>
#include <SDL2/SDL.h>
#include "timer.h"
#include "vector.h"
#include "mesh.h"

class Window
{
public:
    bool running = false;
    int windowWidth;
    int windowHeight;

    /* Configurable options */
    bool drawGrid = true;
    bool drawWireframe = true;
    bool drawWireframeDots = true;
    bool drawFilledTriangles = true;
    bool enableBackfaceCulling = true;
    float modelPosition[3] = {0, 0, -5};
    float modelRotationSpeed[3] = {0.01, 0.01, 0.01};
    float cameraPosition[3] = {0, 0, 0};
    int fovFactor = 400;

private:
    /* Window */
    SDL_Window *window;
    SDL_Renderer *renderer;
    /* Color buffer  */
    uint32_t *colorBuffer;
    SDL_Texture *colorBufferTexture;
    /* Fps */
    int fpsCap = 60;
    bool enableCap = true;
    int screenTicksPerFrame = 1000 / fpsCap;
    /* Timers */
    Timer capTimer;
    /* Custom objects */
    Mesh mesh;
    /* Event Handling */
    SDL_Event event;

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
    void DrawLine(int x0, int y0, int x1, int y1, uint32_t color);
    void DrawTriangle(int x0, int y0, int x1, int y1, int x2, int y2, uint32_t color);
    void DrawFilledTriangle(int x0, int y0, int x1, int y1, int x2, int y2, uint32_t color);
    void SwapIntegers(int *a, int *b);
    void FillFlatBottomTriangle(int x0, int y0, int x1, int y1, int x2, int y2, uint32_t color);
    void FillFlatTopTriangle(int x0, int y0, int x1, int y1, int x2, int y2, uint32_t color);
};

#endif