#include "window.h"
#include <math.h>

Window::~Window()
{
    std::cout << "Destroying Window";

    // Liberar la memoria dinámica
    free(colorBuffer);

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);

    TTF_CloseFont(textFont);
    TTF_Quit();

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

    // Inicialización TTF
    if (TTF_Init() < 0)
    {
        std::cout << "Error initializing SDL_ttf: " << TTF_GetError() << std::endl;
        running = false;
    }

    // Utilizar SDL para preguntar la resolucion maxima del monitor
    SDL_DisplayMode Window_mode;
    SDL_GetCurrentDisplayMode(0, &Window_mode);

    if (isFullscreen)
    {
        windowWidth = Window_mode.w;
        windowHeight = Window_mode.h;
    }

    // Creamos la ventana SDL
    window = SDL_CreateWindow(
        NULL, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        windowWidth, windowHeight, 0); // SDL_WINDOW_BORDERLESS

    if (!window)
    {
        std::cout << "Error creating SDL Window." << std::endl;
        running = false;
    }

    // Creamos el renderizador SDL
    if (enableCap)
    {
        renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    }
    else
    {
        renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    }

    if (!renderer)
    {
        std::cout << "Error creating SDL renderer." << std::endl;
        running = false;
    }

    // font setup
    textFont = TTF_OpenFont("assets/FreeSans.ttf", 16);
    if (!textFont)
    {
        std::cout << "Error loading font: " << TTF_GetError() << std::endl;
        running = false;
    }

    if (isFullscreen)
    {
        SDL_SetWindowFullscreen(window, SDL_WINDOW_FULLSCREEN);
    }

    if (!running)
    {
        std::cout << "Window Init Fail";
    }
}

void Window::Setup()
{
    // Reservar la memoria requerida en bytes para mantener el color buffer
    colorBuffer = static_cast<uint32_t *>(malloc(sizeof(uint32_t) * windowWidth * windowHeight));

    // Crear la textura SDL utilizada para mostrar el color buffer
    colorBufferTexture = SDL_CreateTexture(
        renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, windowWidth, windowHeight);

    // Custom objects
    // Vector3 meshVertices[]{{-1, -1, -1}, {1, -1, -1}, {-1, 1, -1}, {1, 1, -1}, {-1, -1, 1}, {1, -1, 1}, {-1, 1, 1}, {1, 1, 1}};
    // Vector3 meshFaces[]{{1, 0, 2}, {1, 2, 3}, {4, 5, 7}, {4, 7, 6}, {1, 7, 5}, {1, 3, 7}, {4, 2, 0}, {4, 6, 2}, {2, 7, 3}, {2, 6, 7}, {1, 5, 4}, {1, 4, 0}};
    // mesh = Mesh(this, meshVertices, 8, meshFaces, 12);
    mesh = Mesh(this, "assets/cube.obj");
    mesh.SetRotationAmount(0.01, 0.01, 0.01);

    // Start Timer
    fpsTimer.start();
}

void Window::ProcessInput()
{

    fpsTimer.pause(); // Pausar para prevenir congelamiento
    capTimer.pause(); // Pausar para prevenir congelamiento
    SDL_PollEvent(&event);
    fpsTimer.unpause(); // Continuar al reciibir un evento
    capTimer.unpause(); // Pausar para prevenir congelamiento

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

void Window::Update()
{
    // Iniciar el temporizador de cap
    if (enableCap)
        capTimer.start();

    // Old version
    avgFPS = countedFrames / (fpsTimer.getTicks() / 1000.f);
    ++countedFrames;

    // Custom objects update
    mesh.Update();
}

void Window::Render()
{
    // Clear color buffer
    ClearColorBuffer(static_cast<uint32_t>(0xFF0000000));

    // Render the background grid
    DrawGrid(0xFF616161);

    // Custom objects render
    mesh.Render();

    // Late rendering actions
    PostRender();
}

void Window::PostRender()
{
    // Renderizar el color buffer
    RenderColorBuffer();

    // Remove extra precisition and format the fps text
    std::string avgFPSText = std::to_string(avgFPS).substr(0, std::to_string(avgFPS).size() - 4) + " fps";

    // Render FPS
    textSurface = TTF_RenderText_Solid(textFont, avgFPSText.c_str(), textColor);
    if (!textSurface)
    {
        std::cout << "Failed to render text: " << TTF_GetError() << std::endl;
    }

    SDL_Texture *textTexture = SDL_CreateTextureFromSurface(renderer, textSurface);
    SDL_Rect dest = {2, windowHeight - 21, textSurface->w, textSurface->h};
    SDL_RenderCopy(renderer, textTexture, NULL, &dest);

    // Liberación de memoria local
    SDL_FreeSurface(textSurface);
    SDL_DestroyTexture(textTexture);

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

    // if (fpsTimer.getTicks() % screenTicksPerFrame == 0)
    //     SDL_SetWindowTitle(window, ("FPS: " + std::to_string(avgFPS)).c_str());
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
    for (size_t y = 0; y <= windowHeight; y += 10)
    {
        for (size_t x = 0; x <= windowWidth; x += 10)
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
    for (size_t i = 0; i < longestSideLength; i++)
    {
        // Desde el inicio (x0, y0) dibujamos todos los píxeles
        // y vamos redondeando al alza o baja hasta el final
        DrawPixel(
            round(x0 + (xInc * i)),
            round(y0 + (yInc * i)),
            0xFF00FF00);
    }
}

void Window::DrawTriangle(int x0, int y0, int x1, int y1, int x2, int y2, uint32_t color)
{
    DrawLine(x0, y0, x1, y1, color);
    DrawLine(x1, y1, x2, y2, color);
    DrawLine(x2, y2, x0, y0, color);
}