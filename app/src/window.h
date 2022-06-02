#ifndef WINDOW_H
#define WINDOW_H

#include <iostream>
#include <SDL.h>
#include "timer.h"
#include "vector.h"
#include "mesh.h"
#include "matrix.h"
#include "light.h"
#include "camera.h"

class Window
{
public:
    bool running = false;
    int windowWidth;
    int windowHeight;
    int rendererWidth;
    int rendererHeight;
    bool rendererActive;
    bool rendererDragged;

    /* Depth buffer  */
    float* depthBuffer{ nullptr };

    /* Configurable options */
    bool drawGrid = true;
    bool drawWireframe = false;
    bool drawWireframeDots = false;
    bool drawTriangleNormals = false;
    bool drawFilledTriangles = true;
    bool drawTexturedTriangles = true;
    bool enableBackfaceCulling = true;

    /* Model settings */
    float modelScale[3] = {1, 1, 1};
    float modelTranslation[3] = {0, 0, 5};
    float modelRotation[3] = {5.5, 0.5, 0};

    /* Camera and mouse settings */
    Camera camera;
    Matrix4 viewMatrix;
    float cameraPosition[3];
    bool mouseClicked;
    int mousePosition[2];
    int mouseClickPosition[2];

    /* Projection settings */
    float fovFactor = M_PI / (180/60.0f); // 60º in radians
    float fovFactorInGrades = 60;
    float aspectRatio = windowHeight / static_cast<float>(windowWidth);
    float zNear = 0.1, zFar = 100.0;
    Matrix4 projectionMatrix = Matrix4::PerspectiveMatrix(fovFactor, aspectRatio, zNear, zFar);

    /* Light settings */
    Light light = Light{{0, 0, 1} };
    float lightPosition[3]{0,0,1};

    /* DeltaTime*/
    float deltaTime;

private:
    /* Window */
    SDL_Window *window{ nullptr };
    SDL_Renderer *renderer{ nullptr };
    /* Window textures*/
    SDL_Texture* texture{ nullptr };
    SDL_Texture* frameTexture{ nullptr };
    /* Color buffer */
    uint32_t* colorBuffer{ nullptr };
    SDL_Texture *colorBufferTexture{ nullptr };
    /* Fps */
    int fpsCap = 60;
    int screenRefreshRate = fpsCap;
    bool enableCap = true;
    int screenTicksPerFrame = 1000 / fpsCap;
    /* Timers */
    Timer capTimer;
    /* Custom objects */
    Mesh mesh;
    /* Event Handling */
    SDL_Event event{};


public:
    Window() : windowWidth(1280), windowHeight(720), rendererWidth(965), rendererHeight(655) 
    {
        aspectRatio = rendererHeight / static_cast<float>(rendererWidth);
        projectionMatrix = Matrix4::PerspectiveMatrix(fovFactor, aspectRatio, zNear, zFar);
    };

    ~Window();

    void Init();
    void Setup();

    void ProcessInput();
    void Update();
    void Render();
    void PostRender();

    void ClearColorBuffer(uint32_t color);
    void RenderColorBuffer();
    void ClearDepthBuffer();

    SDL_HitTestResult SDLCALL DraggableHitTest(SDL_Window* window, const SDL_Point* pt, void* data);

    void DrawGrid(unsigned int color);
    void DrawPixel(int sx, int sy, unsigned int color);
    void DrawTrianglePixel(int x, int y, Vector4 a, Vector4 b, Vector4 c, float* oneDivW, uint32_t color);
    void DrawTexel(int x, int y, Vector4 a, Vector4 b, Vector4 c, Texture2 t0, Texture2 t1, Texture2 t2, float* uDivW, float* vDivW, float* oneDivW, uint32_t* texture, int textureWidth, int textureHeight);
    void DrawRect(int sx, int sy, int width, int height, uint32_t color);
    void DrawLine(int x0, int y0, int x1, int y1, uint32_t color);
    void DrawLine3D(int x0, int y0, float w0, int x1, int y1, float w1, uint32_t color);
    void DrawTriangle(int x0, int y0, int x1, int y1, int x2, int y2, uint32_t color);
    void DrawTriangle3D(int x0, int y0, float w0, int x1, int y1, float w1, int x2, int y2, float w2, uint32_t color);
    void DrawFilledTriangle(int x0, int y0, float z0, float w0, int x1, int y1, float z1, float w1, int x2, int y2, float z2, float w2, uint32_t color);
    void DrawTexturedTriangle(int x0, int y0, float z0, float w0, Texture2 uv0, int x1, int y1, float z1, float w1, Texture2 uv1, int x2, int y2, float z2, float w2, Texture2 uv2, uint32_t* texture, int textureWidth, int textureHeight);
    void SwapIntegers(int *a, int *b);
    void SwapFloats(float* a, float* b);
    void SwapTextures(Texture2* a, Texture2* b);
    void FillFlatBottomTriangle(int x0, int y0, int x1, int y1, int x2, int y2, uint32_t color);
    void FillFlatTopTriangle(int x0, int y0, int x1, int y1, int x2, int y2, uint32_t color);
};

#endif