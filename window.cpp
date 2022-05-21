#include "window.h"
#include <math.h>
#include "imgui/imgui.h"
#include "imgui/imgui_impl_sdl.h"
#include "imgui/imgui_impl_sdlrenderer.h"

#if !SDL_VERSION_ATLEAST(2, 0, 17)
#error This backend requires SDL 2.0.17+ because of SDL_RenderGeometry() function
#endif

Window::~Window()
{
    std::cout << "Destroying Window";

    // Liberar la memoria dinámica
    free(colorBuffer);

    // Liberamos ImGUI
    ImGui_ImplSDLRenderer_Shutdown();
    ImGui_ImplSDL2_Shutdown();
    ImGui::DestroyContext();

    // Liberamos SDL
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);

    SDL_Quit();
}

void Window::Init()
{
    running = true;

    // Inicializamos SDL
    if (SDL_Init(SDL_INIT_EVERYTHING) != 0)
    {
        std::cout << "Error initializing SDL." << std::endl;
        running = false;
    }

    // Utilizar SDL para preguntar la resolucion maxima del monitor
    SDL_DisplayMode Window_mode;
    SDL_GetCurrentDisplayMode(0, &Window_mode);
    // Set the max FPS as the monitor max hz
    screenRefreshRate = Window_mode.refresh_rate;
    fpsCap = screenRefreshRate;

    // Creamos la ventana SDL
    window = SDL_CreateWindow(NULL, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, windowWidth, windowHeight, 0); // SDL_WINDOW_BORDERLESS

    if (!window)
    {
        std::cout << "Error creating SDL Window." << std::endl;
        running = false;
    }

    // Creamos el renderizador SDL
    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_PRESENTVSYNC | SDL_RENDERER_ACCELERATED);

    if (!renderer)
    {
        std::cout << "Error creating SDL renderer." << std::endl;
        running = false;
    }

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui::StyleColorsDark();
    // Setup Platform/Renderer backends
    ImGui_ImplSDL2_InitForSDLRenderer(window, renderer);
    ImGui_ImplSDLRenderer_Init(renderer);
}

void Window::Setup()
{
    // Reservar la memoria requerida en bytes para mantener el color buffer
    colorBuffer = static_cast<uint32_t *>(malloc(sizeof(uint32_t) * windowWidth * windowHeight));
    // Crear la textura SDL utilizada para mostrar el color buffer
    colorBufferTexture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, windowWidth, windowHeight);
    // Custom objects
    Vector3 meshVertices[]{{-1, -1, -1}, {1, -1, -1}, {-1, 1, -1}, {1, 1, -1}, {-1, -1, 1}, {1, -1, 1}, {-1, 1, 1}, {1, 1, 1}};
    Vector3 meshFaces[]{{2, 1, 3}, {2, 3, 4}, {5, 6, 8}, {5, 8, 7}, {2, 8, 6}, {2, 4, 8}, {5, 3, 1}, {5, 7, 3}, {3, 8, 4}, {3, 7, 8}, {2, 6, 5}, {2, 5, 1}};
    uint32_t meshColors[]{0xFFFF0000, 0xFFFF0000, 0xFF00FF00, 0xFF00FF00, 0xFF0000FF, 0xFF0000FF, 0xFFFFA500, 0xFFFFA500, 0xFFFFFF00, 0xFFFFFF00, 0xFF00FFFF, 0xFF00FFFF};
    mesh = Mesh(this, meshVertices, 8, meshFaces, 12, meshColors);
    // mesh = Mesh(this, "assets/cube.obj");
}

void Window::ProcessInput()
{
    while (SDL_PollEvent(&event))
    {
        ImGui_ImplSDL2_ProcessEvent(&event);
        switch (event.type)
        {
        case SDL_QUIT:
            running = false;
            break;
        case SDL_KEYDOWN:
            if (event.key.keysym.sym == SDLK_ESCAPE)
                running = false;
            break;
        }
    }
}

void Window::Update()
{

    // Iniciar el temporizador de cap
    if (enableCap)
        capTimer.start();

    // Iniciamos un Frame de Imgui
    ImGui_ImplSDLRenderer_NewFrame();
    ImGui_ImplSDL2_NewFrame();
    ImGui::NewFrame();
    // Creamos ventana demo de ImGUI
    ImGui::Begin("CPU 3D Rendering");
    ImGui::Checkbox("Limitar FPS", &this->enableCap);
    ImGui::SliderInt(" ", &this->fpsCap, 5, this->screenRefreshRate);
    ImGui::Checkbox("Dibujar cuadrícula", &this->drawGrid);
    ImGui::Checkbox("Dibujar vértices", &this->drawWireframeDots);
    ImGui::Checkbox("Dibujar wireframe", &this->drawWireframe);
    ImGui::Checkbox("Rellenar triángulos", &this->drawFilledTriangles);
    ImGui::Checkbox("Back-face culling", &this->enableBackfaceCulling);
    ImGui::Separator();
    ImGui::Text("Posición del modelo");
    ImGui::SliderFloat2("Pos", modelPosition, -2, 2);
    ImGui::Text("Velocidad de rotación");
    ImGui::SliderFloat3("Rot", modelRotationSpeed, 0, 0.05f);
    ImGui::Separator();
    // ImGui::Text("Posición cámara (X,Y,Z)");
    // ImGui::SliderFloat3("-5, 5", cameraPosition, -5, 5);
    ImGui::Text("Campo de visión");
    ImGui::SliderInt("Fov", &this->fovFactor, 75, 1000);
    ImGui::SetCursorPosY((ImGui::GetWindowSize().y - 20));
    ImGui::Separator();
    ImGui::Text(" %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
    ImGui::End();

    // Update Model Rotation Speed
    mesh.SetRotationAmount(
        modelRotationSpeed[0], modelRotationSpeed[1], modelRotationSpeed[2]);

    // Update Screen Ticks si han sido mofificados
    screenTicksPerFrame = 1000 / this->fpsCap;

    // Custom objects update
    mesh.Update();
}

void Window::Render()
{
    // Clear color buffer
    ClearColorBuffer(static_cast<uint32_t>(0xFF0000000));

    // Render the background grid
    if (this->drawGrid)
        DrawGrid(0xFF616161);

    // Custom objects render
    mesh.Render();

    // Renderizamos el frame de ImGui
    ImGui::Render();

    // Late rendering actions
    PostRender();
}

void Window::PostRender()
{
    // Renderizar el color buffer
    RenderColorBuffer();

    // Antes de presentar llamamos al SDL Renderer de ImGUI
    ImGui_ImplSDLRenderer_RenderDrawData(ImGui::GetDrawData());

    // Finalmente actualizar la pantalla
    SDL_RenderPresent(renderer);

    // Por último capar los fotogramas si es necesario
    if (enableCap)
    {
        int frameTicks = capTimer.getTicks();
        if (enableCap && frameTicks < screenTicksPerFrame)
        {
            // Esperamos el tiempo restante
            SDL_Delay(screenTicksPerFrame - frameTicks);
        }
    }
}

void Window::ClearColorBuffer(uint32_t color)
{
    for (size_t y = 0; y < windowHeight; y++)
    {
        for (size_t x = 0; x < windowWidth; x++)
        {
            colorBuffer[(windowWidth * y) + x] = color;
        }
    }
}

void Window::RenderColorBuffer()
{
    // Copiar el color buffer y su contenido a la textura
    // Así podremos dibujar la textura en el renderer
    SDL_UpdateTexture(colorBufferTexture, NULL, colorBuffer, windowWidth * sizeof(uint32_t));
    SDL_RenderCopy(renderer, colorBufferTexture, NULL, NULL);
}

void Window::DrawGrid(unsigned int color)
{
    for (size_t y = 0; y < windowHeight; y += 10)
    {
        for (size_t x = 0; x < windowWidth; x += 10)
        {
            colorBuffer[(windowWidth * y) + x] = static_cast<uint32_t>(color);
        }
    }
}

void Window::DrawPixel(int x, int y, unsigned int color)
{
    if (x >= 0 && x <= windowWidth && y >= 0 && y <= windowHeight)
    {
        colorBuffer[(windowWidth * y) + x] = static_cast<uint32_t>(color);
    }
}

void Window::DrawRect(int sx, int sy, int width, int height, uint32_t color)
{
    for (size_t y = sy; (y < sy + height) && (y < windowHeight); y++)
    {
        for (size_t x = sx; (x < sx + width) && (x < windowWidth); x++)
        {
            colorBuffer[(windowWidth * y) + x] = static_cast<uint32_t>(color);
        }
    }
}

void Window::DrawLine(int x0, int y0, int x1, int y1, uint32_t color)
{
    // Calculamos la pendiente m = Δy/Δx
    float dX = x1 - x0;
    float dY = y1 - y0;

    // Definimos la longitud con el mayor lado
    // Si pendiente < 1 tomamos dX (más ancho que alto)
    // Si pendiente >= 1 tomamos dY (más alto que ancho)
    // Nota: Como (float / 0.0) es inf no dará error,
    // incluso siendo la línea completamente vertical
    int longestSideLength = abs(dY / dX) < 1 ? abs(dX) : abs(dY);

    // Buscamos cuanto debemos ir incrementando x e y
    // Uno de ellos siempre será 1 y el otro menor que 1
    float xInc = dX / longestSideLength;
    float yInc = dY / longestSideLength;

    // Dibujamos todos los puntos para el lado más largo
    for (size_t i = 0; i <= longestSideLength; i++)
    {
        // Desde el inicio (x0, y0) dibujamos todos los píxeles
        // y vamos redondeando al alza o baja hasta el final
        DrawPixel(
            round(x0 + (xInc * i)),
            round(y0 + (yInc * i)),
            color);
    }
}

void Window::DrawTriangle(int x0, int y0, int x1, int y1, int x2, int y2, uint32_t color)
{
    DrawLine(x0, y0, x1, y1, color);
    DrawLine(x1, y1, x2, y2, color);
    DrawLine(x2, y2, x0, y0, color);
}

void Window::SwapIntegers(int *a, int *b)
{
    int tmp = *a;
    *a = *b;
    *b = tmp;
}

void Window::DrawFilledTriangle(int x0, int y0, int x1, int y1, int x2, int y2, uint32_t color)
{
    // Reordenamiento de los vértices y0 < y1 < y2
    if (y0 > y1) // Primer intercambio
    {
        SwapIntegers(&y0, &y1);
        SwapIntegers(&x0, &x1);
    }
    if (y1 > y2) // Segundo intercambio
    {
        SwapIntegers(&y1, &y2);
        SwapIntegers(&x1, &x2);
    }
    if (y0 > y1) // Tercer intercambio
    {
        SwapIntegers(&y0, &y1);
        SwapIntegers(&x0, &x1);
    }

    // Si la cara es plana por abajo bypaseamos el triángulo inferior
    if (y1 == y2)
    {
        FillFlatBottomTriangle(x0, y0, x1, y1, x2, y2, color);
    }
    // Si la cara es plana por abajo bypaseamos el triángulo inferior
    else if (y0 == y1)
    {
        FillFlatTopTriangle(x0, y0, x1, y1, x2, y2, color);
    }
    else
    {
        // Calcular el vértice (Mx, My) usando similitudes
        int Mx = (((x2 - x0) * (y1 - y0)) / static_cast<float>((y2 - y0))) + x0;
        int My = y1;

        // Dibujar triángulo con lado inferior plano
        FillFlatBottomTriangle(x0, y0, x1, y1, Mx, My, color);
        // Dibujar triángulo con lado superior plano
        FillFlatTopTriangle(x1, y1, Mx, My, x2, y2, color);
    }
}

void Window::FillFlatBottomTriangle(int x0, int y0, int x1, int y1, int x2, int y2, uint32_t color)
{
    // Algoritmo propio
    float m1 = -((y1 - y0) / static_cast<float>((x0 - x1))); // m1 izquierda -
    float m2 = (y2 - y0) / static_cast<float>((x2 - x0));    // m2 derecha +

    for (int i = 0; i < (y1 - y0); i++)
    {
        DrawLine(x0 + (i / m1), y0 + i, x0 + (i / m2), y0 + i, color);
    }
    /*
    // Algoritmo Gustadolf
    // ===================

    // Calculamos las pendientes pero respecto a la altura que es la que tenemos
    // La pendiente y/x es respecto a x, la pendiente inversa x/y es respecto a y
    // Con este cálculo conseguiremos el incremento exacto por pixel en altura y

    // Para el triángulo izquierdo tomamos la distancia hacia la izquierda y arriba
    float m1 = static_cast<float>((x1 - x0)) / (y1 - y0);
    // Para el triángulo derecho tomamos la distancia hacia la derecha y arriba
    float m2 = static_cast<float>((x2 - x0)) / (y2 - y0);

    float xStart = x0;
    float xEnd = x0;

    for (int y = y0; y < y2; y++)
    {
        DrawLine(xStart, y, xEnd, y, color);
        xStart += m1;
        xEnd += m2;
    } */
}

void Window::FillFlatTopTriangle(int x0, int y0, int x1, int y1, int x2, int y2, uint32_t color)
{
    // Algoritmo propio
    float m1 = -((y2 - y0) / static_cast<float>((x2 - x0))); // m1 izquierda -
    float m2 = -((y2 - y1) / static_cast<float>((x2 - x1))); // m2 izquierda -

    for (int i = 0; i <= (y2 - y1); i++)
    {
        DrawLine(x2 + (i / m1), y2 - i, x2 + (i / m2), y2 - i, color);
    }

    /*
    // Versión de Gustafolf
    // Pendientes inversas
    float m1 = (x2 - x0) / static_cast<float>((y2 - y0));
    float m2 = (x2 - x1) / static_cast<float>((y2 - y1));

    float xStart = x2;
    float xEnd = x2;

    for (int y = y2; y >= y0; y--)
    {
        DrawLine(xStart, y, xEnd, y, color);
        xStart -= m1;
        xEnd -= m2;
    } */
}