#include "window.h"
#include <math.h>
#include "imgui.h"
#include "imgui_impl_sdl.h"
#include "imgui_impl_sdlrenderer.h"

#if !SDL_VERSION_ATLEAST(2, 0, 17)
#error This backend requires SDL 2.0.17+ because of SDL_RenderGeometry() function
#endif

Window::~Window()
{
    std::cout << "Destroying Window";

    // Liberar la memoria dinámica
    free(colorBuffer);
    free(depthBuffer);

    // Liberamos la textura del mesh
    mesh.Free();

    // Liberamos ImGUI
    ImGui_ImplSDLRenderer_Shutdown();
    ImGui_ImplSDL2_Shutdown();
    ImGui::DestroyContext();

    // Liberamos SDL
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);

    SDL_Quit();
}

static SDL_HitTestResult SDLCALL hitTest(SDL_Window* window, const SDL_Point* pt, void* data)
{
    int w, h;
    SDL_GetWindowSize(window, &w, &h);

    const SDL_Rect dragArea = { 70, 0, w-70-25, 20 };

    if (SDL_PointInRect(pt, &dragArea)) {
        //SDL_Log("HIT-TEST: DRAGGABLE\n");
        return SDL_HITTEST_DRAGGABLE;
    }
    //SDL_Log("HIT-TEST: NORMAL\n");
    return SDL_HITTEST_NORMAL;
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
    if (!enableCap)
    {
        fpsCap = screenRefreshRate;
    }

    // Creamos la ventana SDL
    SDL_WindowFlags window_flags = (SDL_WindowFlags)(SDL_WINDOW_BORDERLESS | SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALLOW_HIGHDPI); // SDL_WINDOW_BORDERLESS
    window = SDL_CreateWindow("3D Renderer", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, windowWidth, windowHeight, window_flags);

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

    if (SDL_SetWindowHitTest(window, hitTest, NULL) == -1) {
        SDL_Log("Enabling hit-testing failed!\n");
        running = false;
    }

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    //ImGui::StyleColorsClassic();
    //ImGui::StyleColorsDark();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.ConfigWindowsMoveFromTitleBarOnly = true;

    // Setup Platform/Renderer backends
    ImGui_ImplSDL2_InitForSDLRenderer(window, renderer);
    ImGui_ImplSDLRenderer_Init(renderer);

    // Other options
    SDL_WarpMouseInWindow(window, 495, 370);
    SDL_SetWindowResizable(window, SDL_FALSE);

}

void Window::Setup()
{
    // Reservar la memoria requerida en bytes para mantener el color buffer
    colorBuffer = static_cast<uint32_t *>(malloc(sizeof(uint32_t) * rendererWidth * rendererHeight));
    // Reservar la memoria para el depth buffer
    depthBuffer = static_cast<float*>(malloc(sizeof(float) * rendererWidth * rendererHeight));
    // Crear la textura SDL utilizada para mostrar el color buffer
    colorBufferTexture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA32, SDL_TEXTUREACCESS_STREAMING, rendererWidth, rendererHeight);
    /* Mesh loading */
    //mesh = Mesh(this, "res/f117.obj", "res/f117.png");
    mesh = Mesh(this, "res/cube.obj", "res/cube.png");
    // !!!! Añadir más meshes implicará crear todo el funcionamiento del update y render a nivel global y no en la malla
}

void Window::ProcessInput()
{
    // Update mouse positions for debugging
    SDL_GetMouseState(&mousePosition[0], &mousePosition[1]);

    while (SDL_PollEvent(&event))
    {
        ImGui_ImplSDL2_ProcessEvent(&event);
        switch (event.type)
        {
        case SDL_QUIT:
            running = false;
            break;
        case SDL_KEYDOWN:
            if (event.key.keysym.sym == SDLK_ESCAPE) running = false;
            break;
        case SDL_MOUSEBUTTONDOWN:
            mouseClicked = true;
            break;
        case SDL_MOUSEBUTTONUP:
            mouseClicked = false;
            break;
        case SDL_MOUSEMOTION:
            if (mouseClicked and rendererFocused and !rendererDragged){
                // Rotation per second in radians
                float mouseSensitivity = 0.175;
                // Increment the yaw and the pitch
                camera.yawPitch[0] += event.motion.xrel * mouseSensitivity * deltaTime;
                camera.yawPitch[1] += event.motion.yrel * mouseSensitivity * deltaTime;
                // Clamp the pitch between values close to -90º and 90º (-PI/2 and PI/2) to avoid flipping
                if (camera.yawPitch[1] < (-M_PI / 2 + 0.05)) camera.yawPitch[1] = -M_PI / 2 + 0.05;
                if (camera.yawPitch[1] > (M_PI / 2 - 0.05)) camera.yawPitch[1] = M_PI / 2 - 0.05;
            }
            break;
        case SDL_MOUSEWHEEL:
            if (rendererFocused and rendererHovered and !rendererDragged) {
                // Scroll up
                if (event.wheel.y > 0)
                {
                    camera.forwardVelocity = camera.direction * 30.0 * deltaTime;
                    camera.position += camera.forwardVelocity;
                }
                // Scroll down
                else if (event.wheel.y < 0)
                {
                    camera.forwardVelocity = camera.direction * 30.0 * deltaTime;
                    camera.position -= camera.forwardVelocity;
                }
                // Set the result moving positions into the camera interface
                cameraPosition[0] = camera.position.x;
                cameraPosition[1] = camera.position.y;
                cameraPosition[2] = camera.position.z;
            }
            break;
        }
    }

    // Process the WASD movement with a keyState map
    if (rendererFocused and !rendererDragged)
    {
        const uint8_t* keystate = SDL_GetKeyboardState(NULL);

        // Calculate the forwardVelocity for the z axis and increment it
        int zMovement{ keystate[SDL_SCANCODE_W] - keystate[SDL_SCANCODE_S] };
        if (zMovement != 0)
        {
            camera.forwardVelocity = camera.direction * 5.0 * deltaTime;
            camera.position += camera.forwardVelocity * zMovement;
        }

        // Calculate the sideVelocity for the x axis and increment it
        int xMovement{ keystate[SDL_SCANCODE_A] - keystate[SDL_SCANCODE_D] };
        if (xMovement != 0)
        {
            Vector3 vectorLeft = camera.direction.CrossProduct({ 0, 1, 0 });
            camera.sideVelocity = vectorLeft * 5.0 * deltaTime;
            camera.position += camera.sideVelocity * xMovement;
        }

        // Calculate the sideVelocity for the x axis and increment it
        int yMovement{ keystate[SDL_SCANCODE_E] - keystate[SDL_SCANCODE_Q] };
        if (yMovement != 0)
        {
            Vector3 vectorUp = { 0, 1, 0 };
            camera.verticalVelocity = vectorUp * 5.0 * deltaTime;
            camera.position += camera.verticalVelocity * yMovement;
        }

        // Set the result moving positions into the camera interface
        cameraPosition[0] = camera.position.x;
        cameraPosition[1] = camera.position.y;
        cameraPosition[2] = camera.position.z;
    }
}

void Window::Update()
{
    // Iniciar el temporizador de cap
    if (enableCap) capTimer.start();

    // Start ImGui Frame
    ImGui_ImplSDLRenderer_NewFrame();
    ImGui_ImplSDL2_NewFrame(window);
    ImGui::NewFrame();

    // Top menú bar
    if (ImGui::BeginMainMenuBar())
    {
        if (ImGui::BeginMenu("Menú"))
        {
            if (ImGui::MenuItem("Salir")) { running = false; }
            ImGui::EndMenu();
        }
        ImGui::SameLine();
        ImGui::SetCursorPosX(windowWidth - 20);
        if (ImGui::MenuItem("X")) { running = false; }
        ImGui::SetCursorPosX(windowWidth/2 - 75);
        ImGui::Text("3D Renderer by Hektor");
        ImGui::EndMainMenuBar();
    }

    // Debug window
    ImGui::SetNextWindowSize(ImVec2(275, rendererHeight + 20));
    ImGui::SetNextWindowPos(ImVec2(windowWidth-275-14, 31));
    ImGui::Begin("Debugging", NULL, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoCollapse);
    ImGui::Checkbox("Limitar FPS", &this->enableCap);
    ImGui::SliderInt(" ", &this->fpsCap, 5, this->screenRefreshRate);
    ImGui::Separator();
    ImGui::Text("Rasterizado");
    ImGui::Checkbox("Dibujar triángulos", &this->drawFilledTriangles);
    ImGui::Checkbox("Dibujar texturas", &this->drawTexturedTriangles);
    ImGui::Checkbox("Back-face culling", &this->enableBackfaceCulling);
    ImGui::Separator();
    ImGui::Text("Debugging");
    ImGui::Checkbox("Dibujar cuadrícula", &this->drawGrid);
    ImGui::Checkbox("Dibujar vértices", &this->drawWireframeDots);
    ImGui::Checkbox("Dibujar wireframe", &this->drawWireframe);
    ImGui::Checkbox("Dibujar normales", &this->drawTriangleNormals);
    ImGui::Separator();
    ImGui::Text("Escalado del modelo");
    ImGui::SliderFloat3("Scale", modelScale, 0, 5);
    ImGui::Text("Traslación del modelo");
    ImGui::SliderFloat3("Translate", modelTranslation, -5, 5);
    ImGui::Text("Rotación del modelo");
    ImGui::SliderFloat3("Rotate", modelRotation, 0, 10);
    ImGui::Separator();
    ImGui::Text("Posición cámara (X,Y,Z)");
    ImGui::SliderFloat3("Camera", cameraPosition, -5, 5);
    ImGui::Text("Ángulos cámara (yaw, pitch)");
    ImGui::SliderFloat2("Angles", camera.yawPitch, -5, 5);
    ImGui::Text("Posición ratón (X,Y)");
    ImGui::SliderInt2("Mouse", mousePosition, 0, 0);
    ImGui::Separator();
    ImGui::Text("Luz global (X,Y,Z)");
    ImGui::SliderFloat3("Light", lightPosition, -1, 1);
    ImGui::Separator();
    ImGui::Text("Campo de visión");
    ImGui::SliderFloat("Fov", &this->fovInGrades, 30, 120);
    ImGui::End();

    // Rendering window
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
    ImGui::SetNextWindowSize(ImVec2(rendererWidth, rendererHeight + 20));
    ImGui::SetNextWindowPos(ImVec2(14, 31));
    ImGui::Begin("Rendering", NULL, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoCollapse);
    rendererDragged = ImGui::IsItemHovered();
    ImGui::Image(colorBufferTexture, ImVec2(rendererWidth, rendererHeight));
    rendererFocused = ImGui::IsWindowFocused();
    rendererHovered = ImGui::IsWindowHovered();
    ImGui::SetCursorPosX(10);
    ImGui::SetCursorPosY((ImGui::GetWindowSize().y - 20));
    ImGui::Text("%.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
    ImGui::End();
    ImGui::PopStyleVar();

    // End the Frame
    ImGui::EndFrame();

    // DeltaTime saving
    deltaTime = ImGui::GetIO().DeltaTime;

    // Update Model Settings
    mesh.SetScale(modelScale);
    mesh.SetRotation(modelRotation);
    mesh.SetTranslation(modelTranslation);

    // Update Camera Position
    camera.position = Vector3(cameraPosition[0], cameraPosition[1], cameraPosition[2]);

    // Update the Projection Matrix and thr Frustum
    fovFactorY = M_PI / (180 / fovInGrades);  // in radians
    fovFactorX = 2 * atan(tan(fovFactorY / 2) * aspectRatioX);  // in radians
    projectionMatrix = Matrix4::PerspectiveMatrix(fovFactorY, aspectRatioY, zNear, zFar);
    viewFrustum = Frustum(fovFactorX, fovFactorY, zNear, zFar);

    // Update Screen Ticks si han sido mofificados
    screenTicksPerFrame = 1000 / this->fpsCap;

    // Update the light position
    light.direction = Vector3(lightPosition[0], lightPosition[1], lightPosition[2]);

    // Custom objects update
    mesh.Update();
}

void Window::Render()
{

    // Clear color buffer
    ClearColorBuffer(static_cast<uint32_t>(0xFF404040));

    // Render the background grid
    if (this->drawGrid) DrawGrid(0xFF616161);

    // Custom objects render
    mesh.Render();

    // Renderizamos el frame de ImGui
    ImGui::Render();

    // Clear the renderer
    SDL_SetRenderDrawColor(renderer, 80, 80, 80, 0);
    SDL_RenderClear(renderer);

    // Late rendering actions
    PostRender();
}

void Window::PostRender()
{

    // Renderizar el color buffer
    RenderColorBuffer();

    // Antes de presentar llamamos al SDL Renderer de ImGUI
    ImGui_ImplSDLRenderer_RenderDrawData(ImGui::GetDrawData());

    // Clear depth buffer rendeer present
    ClearDepthBuffer();

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
    for (size_t y = 0; y < rendererHeight; y++)
    {
        for (size_t x = 0; x < rendererWidth; x++)
        {
            colorBuffer[(rendererWidth * y) + x] = color;
        }
    }
}

void Window::ClearDepthBuffer()
{
    for (size_t y = 0; y < rendererHeight; y++)
    {
        for (size_t x = 0; x < rendererWidth; x++)
        {
            depthBuffer[(rendererWidth * y) + x] = 1.0;
        }
    }
}

void Window::RenderColorBuffer()
{
    // Copiar el color buffer y su contenido a la textura de imgui
    // Así podremos dibujar la textura en el renderer
    SDL_UpdateTexture(colorBufferTexture, NULL, colorBuffer, rendererWidth * sizeof(uint32_t));
    //SDL_RenderCopy(renderer, colorBufferTexture, NULL, NULL);
}

void Window::DrawGrid(unsigned int color)
{
    for (size_t x = 72; x < rendererWidth; x += 100)
    {
        for (size_t y = 0; y < rendererHeight; y++) {
            DrawPixel(x, y, color);
        }
    }

    for (size_t y = 72; y < rendererHeight; y+= 100)
    {
        for (size_t x = 0; x < rendererWidth; x++) {
            DrawPixel(x, y, color);
        }
    }
}

void Window::DrawPixel(int x, int y, unsigned int color)
{
    if (x >= 0 && x < rendererWidth && y >= 0 && y < rendererHeight)
    {
        colorBuffer[(rendererWidth * y) + x] = static_cast<uint32_t>(color);
    }
}

void Window::DrawTexel(int x, int y, Vector4 a, Vector4 b, Vector4 c, Texture2 t0, Texture2 t1, Texture2 t2, float *uDivW, float* vDivW, float* oneDivW, uint32_t *texture, int textureWidth, int textureHeight)
{
    // Create p vector with current pixel location
    Vector2 p{ static_cast<double>(x),static_cast<double>(y) };
    // Calculate the weights using the vectors A,B,C and P
    Vector3 weights = Vector3::BarycentricWeights(a.ToVector2(), b.ToVector2(), c.ToVector2(), p);
    float alpha = weights.x;
    float beta = weights.y;
    float gamma = weights.z;

    // Variables to store the interpolated values of U, V and also the reciprocal 1/w for the current pixel
    float interpolatedU;
    float interpolatedV;
    float interpolatedReciprocalW;

    // Calculate the interpolations multipling every U/w and V/w coord per its weight factor per 1/w
    interpolatedU = uDivW[0] * alpha + uDivW[1] * beta + uDivW[2] * gamma;
    interpolatedV = vDivW[0] * alpha + vDivW[1] * beta + vDivW[2] * gamma;

    // Find the interpolate value of 1/w for the current pixel
    interpolatedReciprocalW = oneDivW[0] * alpha + oneDivW[1] * beta + oneDivW[2] * gamma;

    // Now we can divide back both interpolated values by 1/w
    interpolatedU /= interpolatedReciprocalW;
    interpolatedV /= interpolatedReciprocalW;

    // Calculate the texelX and texelY based on the interpolated UV and the texture sizes
    int texelX = abs(static_cast<int>(interpolatedU * textureWidth)) % textureWidth;
    int texelY = abs(static_cast<int>(interpolatedV * textureHeight)) % textureHeight;

    // Adjust the reciprocal 1/w to the contrary distance. E.g. 0.1 -> 0.9
    interpolatedReciprocalW = 1 - interpolatedReciprocalW;

    // Security check to not draw outside the buffers size
    int bufferPosition = this->rendererWidth * y + x;
    if (bufferPosition >= 0 && bufferPosition <= (this->rendererWidth * this->rendererHeight)) {
        // Only draw the pixel if the depth value is less than the one previously stored in the depth buffer
        if (interpolatedReciprocalW < this->depthBuffer[(this->rendererWidth * y) + x])
        {
            // Finally draw the pixel with the color stored in our texture harcoded array
            DrawPixel(x, y, texture[(textureWidth * texelY) + texelX]);

            // And update the depth for the pixel in the depthBuffer
            this->depthBuffer[(this->rendererWidth * y) + x] = interpolatedReciprocalW;
        }
    }
}

void Window::DrawTrianglePixel(int x, int y, Vector4 a, Vector4 b, Vector4 c, float* oneDivW, uint32_t color)
{
    // Create p vector with current pixel location
    Vector2 p{ static_cast<double>(x),static_cast<double>(y) };
    // Calculate the weights using the vectors A,B,C and P
    Vector3 weights = Vector3::BarycentricWeights(a.ToVector2(), b.ToVector2(), c.ToVector2(), p);
    float alpha = weights.x;
    float beta = weights.y;
    float gamma = weights.z;

    // Variables to store the interpolated values of U, V and also the reciprocal 1/w for the current pixel
    float interpolatedReciprocalW;

    // Find the interpolate value of 1/w for the current pixel
    interpolatedReciprocalW = oneDivW[0] * alpha + oneDivW[1] * beta + oneDivW[2] * gamma;

    // Adjust the reciprocal 1/w to the contrary distance. E.g. 0.1 -> 0.9
    interpolatedReciprocalW = 1 - interpolatedReciprocalW;

    // Security check to not draw outside the buffers size
    int bufferPosition = (rendererWidth * y) + x;
    if (bufferPosition >= 0 && bufferPosition <= (this->rendererWidth * this->rendererHeight)) {
        // Only draw the pixel if the depth value is less than the one previously stored in the depth buffer
        if (interpolatedReciprocalW < this->depthBuffer[bufferPosition])
        {
            // Finally draw the pixel with the solid color
            DrawPixel(x, y, color);

            // And update the depth for the pixel in the depthBuffer
            this->depthBuffer[bufferPosition] = interpolatedReciprocalW;
        }
    }
}

void Window::DrawRect(int sx, int sy, int width, int height, uint32_t color)
{
    for (size_t y = sy; (y < sy + static_cast<__int64>(height)) && (y < rendererHeight); y++)
    {
        for (size_t x = sx; (x < sx + static_cast<__int64>(width)) && (x < rendererWidth); x++)
        {
            DrawPixel(x, y, color);
        }
    }
}

void Window::DrawLine(int x0, int y0, int x1, int y1, uint32_t color)
{
    // Calculamos la pendiente m = Δy/Δx
    float dX = x1 - x0;
    float dY = y1 - y0;
    if (abs(dX) == 0 && abs(dY) == 0) return;

    // Definimos la longitud con el mayor lado
    // Si pendiente < 1 tomamos dX (más ancho que alto)
    // Si pendiente >= 1 tomamos dY (más alto que ancho)
    // Nota: Como (float / 0.0) es inf no dará error,
    // incluso siendo la línea completamente vertical
    int longestSideLength = abs(dX / dY) > 1 ? abs(dX) : abs(dY);

    // Buscamos cuanto debemos ir incrementando x e y
    // Uno de ellos siempre será 1 y el otro menor que 1
    float xInc = dX / longestSideLength;
    float yInc = dY / longestSideLength;

    // Dibujamos todos los puntos para el lado más largo
    for (size_t i = 0; i <= longestSideLength; i++)
    {
        // Desde el inicio (x0, y0) dibujamos todos los píxeles
        // y vamos redondeando al alza o baja hasta el final
        DrawPixel(round(x0 + (xInc * i)), round(y0 + (yInc * i)), color);
    }
}

/*void Window::DrawLine3D(int x0, int y0, float w0, int x1, int y1, float w1, uint32_t color)
{
    float deltaX = x1 - x0;
    float deltaY = y1 - y0;
    int deltaReciprocalW = 1.f / w1 - 1.f / w0;
    if (abs(deltaX) == 0 && abs(deltaY) == 0) return;

    int longestSideLength = abs(deltaX / deltaY) > 1 ? abs(deltaX) : abs(deltaY);
    float xInc = deltaX / longestSideLength;
    float yInc = deltaY / longestSideLength;
    float wInc = deltaReciprocalW / static_cast<float>(longestSideLength);

    float currentX = x0;
    float currentY = y0;
    float currentW = 1.f / w0;

    // Dibujamos todos los puntos para el lado más largo
    for (size_t i = 0; i <= longestSideLength; i++)
    {
        int x = roundf(currentX);
        int y = roundf(currentY);
        float oneOverW = currentW;
        float zInterpolated = 1.0f - oneOverW;

        // Security check
        int bufferPosition = rendererWidth * y + x;
        if (bufferPosition >= 0 && bufferPosition <= (this->rendererWidth * this->rendererHeight)) {
            if (zInterpolated < depthBuffer[(y * rendererHeight) + x])
            {
                DrawPixel(x, y, color);
                depthBuffer[(y * rendererHeight) + x] = zInterpolated;
            }
        }

        currentX += xInc;
        currentY += yInc;
        currentW += wInc;
    }
}

void Window::DrawLine3D(int x0, int y0, float w0, int x1, int y1, float w1, uint32_t color)
{
    float deltaX = x1 - x0;
    float deltaY = y1 - y0;
    int deltaReciprocalW = 1 / w1 - 1 / w0;
    if (abs(deltaX) == 0 && abs(deltaY) == 0) return;

    int longestSideLength = abs(deltaX) > abs(deltaY) ? abs(deltaX) : abs(deltaY);
    float xInc = deltaX / static_cast<float>(longestSideLength);
    float yInc = deltaY / static_cast<float>(longestSideLength);
    float wInc = deltaReciprocalW / static_cast<float>(longestSideLength);

    float currentX = x0;
    float currentY = y0;
    float currentW = 1.f / w0;

    // Dibujamos todos los puntos para el lado más largo
    for (size_t i = 0; i <= longestSideLength; i++)
    {
        int x = round(currentX);
        int y = round(currentY);
        float oneOverW = currentW;
        float zInterpolated = 1.f - oneOverW;

        // Security check
        int bufferPosition = (rendererWidth * y) + x;
        if (bufferPosition >= 0 && bufferPosition <= (this->rendererWidth * this->rendererHeight)) {
            if (zInterpolated < depthBuffer[bufferPosition])
            {
                DrawPixel(x, y, color);
                depthBuffer[bufferPosition] = zInterpolated;
            }
        }
        currentX += xInc;
        currentY += yInc;
        currentW += wInc;
    }
}*/

void Window::DrawLine3D(int x0, int y0, float w0, int x1, int y1, float w1, uint32_t color)
{
    // Calculamos la distancia entre X, Y y la recíproca de W
    float deltaX = x1 - x0;
    float deltaY = y1 - y0;
    int deltaReciprocalW = 1.f / w1 - 1.f / w0;

    // Si no hay distancia no hace falta dibujar nada
    if (abs(deltaX) == 0 && abs(deltaY) == 0) return;

    // Buscamos que lado es mayor, el ancho o el alto
    int longestSideLength = abs(deltaX / deltaY) > 1 ? abs(deltaX) : abs(deltaY);

    // Calculamos el incremento por píxel para X, Y y la recíproca de W
    float xInc = deltaX / longestSideLength;
    float yInc = deltaY / longestSideLength;
    float wInc = deltaReciprocalW / static_cast<float>(longestSideLength);

    // Dibujamos todos los puntos para el lado más largo
    for (size_t i = 0; i <= longestSideLength; i++)
    {
        int x = roundf(x0 + (xInc * i));
        int y = roundf(y0 + (yInc * i));
        float oneOverW = 1.0 / (w0 + (wInc * i));
        float zInterpolated = 1.0f - oneOverW;

        // Security check
        int bufferPosition = (rendererWidth * y) + x;
        if (bufferPosition >= 0 && bufferPosition <= (this->rendererWidth * this->rendererHeight)) {
            // Si el valor en Z es menor que el del bufer es que está más cerca
            if (zInterpolated < depthBuffer[bufferPosition])
            {
                DrawPixel(x, y, color);
                depthBuffer[bufferPosition] = zInterpolated;
            }
        }
    }
}

void Window::DrawTriangle(int x0, int y0, int x1, int y1, int x2, int y2, uint32_t color)
{
    DrawLine(x0, y0, x1, y1, color);
    DrawLine(x1, y1, x2, y2, color);
    DrawLine(x2, y2, x0, y0, color);
}

void Window::DrawTriangle3D(int x0, int y0, float w0, int x1, int y1, float w1, int x2, int y2, float w2, uint32_t color)
{
    DrawLine3D(x0, y0, w0, x1, y1, w1, color);
    DrawLine3D(x1, y1, w1, x2, y2, w2, color);
    DrawLine3D(x2, y2, w2, x0, y0, w0, color);
}

void Window::SwapIntegers(int* a, int* b)
{
    int tmp = *a;
    *a = *b;
    *b = tmp;
}

void Window::SwapFloats(float* a, float* b)
{
    float tmp = *a;
    *a = *b;
    *b = tmp;
}

void Window::SwapTextures(Texture2* a, Texture2* b)
{
    Texture2 tmp = *a;
    *a = *b;
    *b = tmp;
}

void Window::DrawTexturedTriangle(int x0, int y0, float z0, float w0, Texture2 uv0, int x1, int y1, float z1, float w1, Texture2 uv1, int x2, int y2, float z2, float w2, Texture2 uv2, uint32_t* texture, int textureWidth, int textureHeight)
{
    // Iterar todos los píxeles del triángulo para renderizarlos en función del color de la textura

    // Reordenamiento de los vértices y las UV coords: y0 < y1 < y2
    if (y0 > y1) // Primer intercambio
    {
        SwapIntegers(&y0, &y1);
        SwapIntegers(&x0, &x1);
        SwapFloats(&z0, &z1);
        SwapFloats(&w0, &w1);
        SwapTextures(&uv0, &uv1);
    }
    if (y1 > y2) // Segundo intercambio
    {
        SwapIntegers(&y1, &y2);
        SwapIntegers(&x1, &x2);
        SwapFloats(&z1, &z2);
        SwapFloats(&w1, &w2);
        SwapTextures(&uv1, &uv2);
    }
    if (y0 > y1) // Tercer intercambio
    {
        SwapIntegers(&y0, &y1);
        SwapIntegers(&x0, &x1);
        SwapFloats(&z0, &z1);
        SwapFloats(&w0, &w1);
        SwapTextures(&uv0, &uv1);
    }

    // Flip the V component to account for inverted UV-coordinates
    uv0.v = 1 - uv0.v;
    uv1.v = 1 - uv1.v;
    uv2.v = 1 - uv2.v;

    // Create vector points for texturing after sorting the vertices
    Vector4 pA{ (double)x0, (double)y0, (double)z0, (double)w0 };
    Vector4 pB{ (double)x1, (double)y1, (double)z1, (double)w1 };
    Vector4 pC{ (double)x2, (double)y2, (double)z2, (double)w2 };

    // Common divisions for texel drawing in all the triangle face
    float uDivW[3] = { uv0.u / pA.w , uv1.u / pB.w, uv2.u / pC.w };
    float vDivW[3] = { uv0.v / pA.w , uv1.v / pB.w, uv2.v / pC.w };
    float oneDivW[3] = { 1 / pA.w , 1 / pB.w, 1 / pC.w };

    /*** Render the upper part of the triangle (flat bottom) ***/
    {
        float m1 = 0;
        float m2 = 0;

        // Checks to avoid infinite divisions
        if (y1 - y0 != 0) m1 = -((y1 - y0) / static_cast<float>((x0 - x1))); // m1 izquierda -
        if (y2 - y0 != 0) m2 = (y2 - y0) / static_cast<float>((x2 - x0));    // m2 derecha +
        if (y1 - y0 != 0)
        {
            for (size_t i = 0; i < (y1 - y0); i++)
            {
                int xStart = x0 + (i / m1);
                int xEnd = x0 + (i / m2);
                int y = y0 + i;

                // Sometimes we have to draw the triangle from right to left
                // so we have to swap the xStart and the xEnd
                if (xEnd < xStart) SwapIntegers(&xEnd, &xStart);

                for (int x = xStart; x < xEnd; x++)
                {
                    //DrawPixel(x, y, (x % 2 == 0) ? 0xFFFFF00FF : 0xFF000000);
                    DrawTexel(x, y, pA, pB, pC, uv0, uv1, uv2, uDivW, vDivW, oneDivW, texture, textureWidth, textureHeight);
                }
            }
        }
    }

    /*** Render the lower part of the triangle (flat top) ***/
    {
        float m1 = 0;
        float m2 = 0;
        // Checks to avoid infinite divisions
        if (y2 - y1 != 0) m1 = -((y2 - y1) / static_cast<float>((x2 - x1))); // m1 izquierda -
        if (y2 - y0 != 0) m2 = -((y2 - y0) / static_cast<float>((x2 - x0))); // m2 izquierda -
        if (y2 - y1 != 0)
        {
            for (size_t i = 0; i <= (y2 - y1); i++)
            {
                int xStart = x2 + (i / m1);
                int xEnd = x2 + (i / m2);
                int y = y2 - i;

                // Sometimes we have to draw the triangle from right to left
                // so we have to swap the xStart and the xEnd
                if (xEnd < xStart) SwapIntegers(&xEnd, &xStart);

                for (int x = xStart; x < xEnd; x++)
                {
                    //DrawPixel(x, y, (x%2 ==0) ? 0xFFFFF00FF : 0xFF000000);
                    DrawTexel(x, y, pA, pB, pC, uv0, uv1, uv2, uDivW, vDivW, oneDivW, texture, textureWidth, textureHeight);
                }
            }
        }
    }
}

void Window::DrawFilledTriangle(int x0, int y0, float z0, float w0, int x1, int y1, float z1, float w1, int x2, int y2, float z2, float w2, uint32_t color)
{
    // Iterar todos los píxeles del triángulo para renderizarlos en función del color de la textura
    if (y0 > y1) // Primer intercambio
    {
        SwapIntegers(&y0, &y1);
        SwapIntegers(&x0, &x1);
        SwapFloats(&z0, &z1);
        SwapFloats(&w0, &w1);
    }
    if (y1 > y2) // Segundo intercambio
    {
        SwapIntegers(&y1, &y2);
        SwapIntegers(&x1, &x2);
        SwapFloats(&z1, &z2);
        SwapFloats(&w1, &w2);
    }
    if (y0 > y1) // Tercer intercambio
    {
        SwapIntegers(&y0, &y1);
        SwapIntegers(&x0, &x1);
        SwapFloats(&z0, &z1);
        SwapFloats(&w0, &w1);
    }

    // Create vector points for texturing after sorting the vertices
    Vector4 pA{ (double)x0, (double)y0, (double)z0, (double)w0 };
    Vector4 pB{ (double)x1, (double)y1, (double)z1, (double)w1 };
    Vector4 pC{ (double)x2, (double)y2, (double)z2, (double)w2 };

    // Common divisions for depth calculations
    float oneDivW[3] = { 1 / pA.w , 1 / pB.w, 1 / pC.w };

    /*** Render the upper part of the triangle (flat bottom) ***/
    {
        float m1 = 0;
        float m2 = 0;

        // Checks to avoid infinite divisions
        if (y1 - y0 != 0) m1 = -((y1 - y0) / static_cast<float>((x0 - x1))); // m1 izquierda -
        if (y2 - y0 != 0) m2 = (y2 - y0) / static_cast<float>((x2 - x0));    // m2 derecha +
        if (y1 - y0 != 0)
        {

            for (size_t i = 0; i < (y1 - y0); i++)
            {
                int xStart = x0 + (i / m1);
                int xEnd = x0 + (i / m2);
                int y = y0 + i;
                
                // Sometimes we have to draw the triangle from right to left
                // so we have to swap the xStart and the xEnd
                if (xEnd < xStart) SwapIntegers(&xEnd, &xStart);

                for (int x = xStart; x < xEnd; x++)
                {
                    uint32_t newColor = color;
                    //if (xStart == 0) newColor = 0xFF000000;
                    DrawTrianglePixel(x, y, pA, pB, pC, oneDivW, newColor);
                }
            }
        }
    }

    /*** Render the lower part of the triangle (flat top) ***/
    {
        float m1 = 0;
        float m2 = 0;
        // Checks to avoid infinite divisions
        if (y2 - y1 != 0) m1 = -((y2 - y1) / static_cast<float>((x2 - x1))); // m1 izquierda -
        if (y2 - y0 != 0) m2 = -((y2 - y0) / static_cast<float>((x2 - x0))); // m2 izquierda -
        if (y2 - y1 != 0)
        {
            for (size_t i = 0; i <= (y2 - y1); i++)
            {
                int xStart = x2 + (i / m1);
                int xEnd = x2 + (i / m2);
                int y = y2 - i;

                // Sometimes we have to draw the triangle from right to left
                // so we have to swap the xStart and the xEnd
                if (xEnd < xStart) SwapIntegers(&xEnd, &xStart);

                for (int x = xStart; x < xEnd; x++)
                {
                    uint32_t newColor = color;
                    // if (xStart == 0) newColor = 0xFF000000;
                    DrawTrianglePixel(x, y, pA, pB, pC, oneDivW, newColor);
                }
            }
        }
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