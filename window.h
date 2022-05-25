#ifndef WINDOW_H
#define WINDOW_H

#include <iostream>
#include <SDL2/SDL.h>
#include "timer.h"
#include "vector.h"
#include "mesh.h"
#include "matrix.h"
#include "light.h"

class Window
{
public:
    bool running = false;
    int windowWidth;
    int windowHeight;

    /* Configurable options */
    bool drawGrid = false;
    bool drawWireframe = false;
    bool drawWireframeDots = false;
    bool drawFilledTriangles = true;
    bool enableBackfaceCulling = true;

    /* Model settings */
    float modelScale[3] = {1, 1, 1};
    float modelTranslation[3] = {1, 0, 0};
    float modelRotation[3] = {0, 0, 0};

    /* Camera settings */
    float cameraPosition[3] = {0, 0, -5};

    /* Projection settings */
    float fovFactor = M_PI / 3.0; // 60º in radians
    float fovFactorInGrades = 70;
    float aspectRatio = windowHeight / static_cast<float>(windowWidth);
    float zNear = 0.1, zFar = 100.0;
    Matrix4 projectionMatrix = Matrix4::PerspectiveMatrix(fovFactor, aspectRatio, zNear, zFar);

    /* Light settings */
    Light light{.direction{0, 0, 1}};

private:
    /* Window */
    SDL_Window *window;
    SDL_Renderer *renderer;
    /* Color buffer  */
    uint32_t *colorBuffer;
    SDL_Texture *colorBufferTexture;
    /* Fps */
    int fpsCap = 30;
    int screenRefreshRate = fpsCap;
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