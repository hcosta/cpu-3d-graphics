# CPU 3D Graphics en C++

La meta de este proyecto es entender el pipeline del renderizado 3D sin utilizar la GPU, implementando en C++ todo el sistema desde cero.

Se utiliza SDL2 como biblioteca multiplataforma para manejar el hardware del sistema.

## Índice

* [Configuración previa](#configuración-previa)
* [Buffer de color](#buffer-de-color)
    * [Dibujar FPS y caparlos](#dibujar-fps-y-caparlos)
    * [Refactorización 1](#refactorización-1)
* [Vectores y puntos](#vectores-y-puntos)
* [Proyección de puntos](#proyección-de-puntos)
    * [Proyección ortográfica](#proyección-ortográfica)
    * [Proyección perspectiva](#proyección-perspectiva)
    * [Regla de la mano](#regla-de-la-mano)
* [Transformaciones lineales](#transformaciones-lineales)
    * [Transformando vectores](#transformando-vectores)
    * [Razones trigonométricas](#razones-trigonométricas)
    * [Rotación de vectores](#rotación-de-vectores)
    * [Función de rotación](#función-de-rotación)
    * [Refactorización 2](#refactorización-2)
* [Triángulos y vértices](#triángulos-y-vértices)
    * [Programando los triángulos](#programando-los-triángulos)


## Configuración previa

Este proyecto se desarrolla en Windows 11 con Visual Studio Code. La estructura principal es:

* `Makefile`
* `main.cpp`
* `src/include`
* `src/lib`
* `bin/SDL2.dll`

El fichero `main.cpp` inicialmente contiene un "Hola mundo" en SDL que intenta crear una ventana vacía:

```cpp
#include <iostream>
#include <SDL2/SDL.h>

const int WIDTH = 640, HEIGHT = 480;

int main(int argc, char *argv[])
{
    SDL_Init(SDL_INIT_EVERYTHING);

    SDL_Window *window = SDL_CreateWindow("Hello SDL WORLD", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, WIDTH, HEIGHT, SDL_WINDOW_ALLOW_HIGHDPI);

    if (NULL == window)
    {
        std::cout << "Could not create window: " << SDL_GetError() << std::endl;
        return 1;
    }

    SDL_Event windowEvent;

    while (true)
    {
        if (SDL_PollEvent(&windowEvent))
        {
            if (SDL_QUIT == windowEvent.type)
            {
                break;
            }
        }
    }

    SDL_DestroyWindow(window);
    SDL_Quit();

    return EXIT_SUCCESS;
}
```

Con el compiladir [GNU GCC Compiler](https://gcc.gnu.org/) instalado en el equipo y la biblioteca [SDL2 x86_64-w64-mingw32](http://libsdl.org/), con `include/SDL2` y `lib` en `src/` y `bin/SDL2.dll` donde el ejecutable `bin/main.exe` para su uso.

La compilación se realiza mediante la herramienta `make`, se puede instalar con [Chocolatey](https://chocolatey.org/):

```
choco install make
```

El `Makefile` contiene la vinculación necesaria de bibliotecas de SDL2:

```makefile
build:
	g++ -I src/include -L src/lib -o bin/main *.cpp -lmingw32 -lSDL2main -lSDL2
run:
	./bin/main.exe
clean:
	rm ./bin/main.exe
```

La compilación y ejecución es tan sencilla como ejecutar:

```
> make
> make run
```

Para facilitar la compilación y ejecución utilizo el complemento [Code Runner](https://marketplace.visualstudio.com/items?itemName=formulahendry.code-runner) de VSC y un comando personalizado definido en el `.vscode/settings.json`:

```json
{
    "code-runner.customCommand": "make && ./bin/main.exe"
}
```

Ejecutamos con `F1 > Run Custom Command` y *voilà*:

![](./docs/image-1.png)

## Buffer de color

El buffer de color es un array de colores (en la práctica `uint32_t`) que maneja cada uno de los píxeles de la pantalla y su color. 

La variable que gestiona este buffer es el puntero `color_buffer`, donde se reserva memoria dinámicamente con `malloc`. El tamaño es el número de píxeles de la pantalla (ancho * alto):

```cpp
uint32_t *color_buffer;
color_buffer = static_cast<uint32_t *>(malloc(sizeof(uint32_t) * window_width * window_height));
```

Para dibujar ese `color_buffer` se tiene que copiar a una textura `color_buffer_texture`:

```cpp
SDL_Texture *color_buffer_texture;
color_buffer_texture = SDL_CreateTexture( renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, window_width, window_height);
```

El proceso de copia del `color_buffer` a la textura y al renderer ocurrirá en cada fotograma:

```cpp
void render_color_buffer()
{
    SDL_UpdateTexture(color_buffer_texture, NULL, color_buffer, window_width * sizeof(uint32_t));
    SDL_RenderCopy(renderer, color_buffer_texture, NULL, NULL);
}
```

El proceso de renderizado empieza con un color de base para limpiar la pantalla, luego una limpieza y renderizado del `color_buffer` y finalmente la actualización de la pantalla:

```cpp
void render()
{
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    SDL_RenderClear(renderer);

    clear_color_buffer(0xFFF00FFFF);
    render_color_buffer();

    SDL_RenderPresent(renderer);
}
```

La limpieza del `color_buffer` se basa en recorrer todos los píxeles del array y establecer el color pasado a la función:

```cpp
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
```

En este punto podemos crear diferentes funciones para cambiar dinámicamente los colores del `color_buffer`, por ejemplo para generar una cuadrícula `draw_grid()`:

```cpp
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
```

![](./docs/image-2.png)

O una función para dibujar rectángulos rellenos de colores `draw_rectangle()`:

```cpp
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
```

![](./docs/image-3.png)

### Dibujar FPS y caparlos

He decidido añadir una opción para dibujar la media de FPS. Para ello necesitaré dibujar un texto en pantalla con la biblioteca [SDL_ttf](https://github.com/libsdl-org/SDL_ttf). 

La versión utilizada en el proyecto es [SDL2_ttf-devel-2.0.18 x86_64-w64-mingw32](https://github.com/libsdl-org/SDL_ttf/releases/download/release-2.0.18/SDL2_ttf-devel-2.0.18-mingw.tar.gz), cuyos `include` y `lib` van al directorio `src` y la DLL `SDL2_ttf.dll` al directorio `bin`.

Una vez hecho se puede importar para utilizarlo:

```cpp
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
```

Para dibujar el texto hay que seguir varios pasos.

En primer lugar se necesita una superficie para renderizar el texto, un color y una fuente que deberemos cargar luego:

```cpp
SDL_Surface *text;
TTF_Font *font;
SDL_Color color = {255, 255, 255};
```

Deberemos inicializar el módulo TTF:

```cpp
if (TTF_Init() < 0)
{
    std::cout << "Error initializing SDL_ttf: " << TTF_GetError() << std::endl;
    return false;
}
```

También durante la inicialización si todo es correcto cargaremos la configuración de la fuente que debemos tener en algún directorio del proyecto:

```cpp
// font setup
font = TTF_OpenFont("assets/FreeSans.ttf", 16);
if (!font)
{
    std::cout << "Error loading font: " << TTF_GetError() << std::endl;
    return false;
}
```

Durante el renderizado la idea es renderizar el texto deseado en la superficie:

```cpp
text = TTF_RenderText_Solid(font, "Hola mundo!", color);
if (!text)
{
    std::cout << "Failed to render text: " << TTF_GetError() << std::endl;
}
```

Justo a continuación renderizaremos la superficie como una textura, configuraremos el tamaño de la recta ed destino y realizaremos la copia al `renderer`:

```cpp
SDL_Texture *text_texture = SDL_CreateTextureFromSurface(renderer, text);
SDL_Rect dest = {2, 459, text->w, text->h};
SDL_RenderCopy(renderer, text_texture, NULL, &dest);
```

Después de actualizar la pantalla podemos liberar de la memoria la textura y la superficie (es muy importante):

```cpp
// Liberación de memoria local
SDL_DestroyTexture(text_texture);
SDL_FreeSurface(textSurface);
```

También deberemos liberar la memoria de la fuente y el módulo TTF al destruir la ventana:

```cpp
TTF_CloseFont(font);
TTF_Quit();
```

Con esto ya tendremos nuestro texto en pantalla:

![](./docs/image-4.png)

Ahora debemos idear una forma de limitar los FPS en caso de que no queramos tener la sincronización vertical activa:

```cpp
renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
```

Siguiendo la idea del tutorial de [LazyFoo](https://lazyfoo.net/tutorials/SDL/25_capping_frame_rate/index.php) he decidido crear mis propia clase `Timer` para tener unos objetos más sofisticados.

La cabecera `timer.h` contiene las definiciones:

```cpp
#ifndef TIMER_H
#define TIMER_H

#include <iostream>
#include <SDL2/SDL.h>

// The application time based timer
class Timer
{
public:
    // Initializes variables
    Timer();

    // The various clock actions
    void start();
    void stop();
    void pause();
    void unpause();

    // Gets the timer's time
    uint32_t getTicks();

    // Checks the status of the timer
    bool isStarted();
    bool isPaused();

private:
    // The clock time when the timer started
    uint32_t mStartTicks;

    // The ticks stored when the timer was paused
    uint32_t mPausedTicks;

    // The timer status
    bool mPaused;
    bool mStarted;
};

#endif
```

Y el fichero de fuentes `timer.cpp` la implementación de la clase:

```cpp
#include "timer.h"

Timer::Timer()
{
    // Initialize the variables
    mStartTicks = 0;
    mPausedTicks = 0;

    mPaused = false;
    mStarted = false;
}

void Timer::start()
{
    // Start the timer
    mStarted = true;

    // Unpause the timer
    mPaused = false;

    // Get the current clock time
    mStartTicks = SDL_GetTicks();
    mPausedTicks = 0;
}

void Timer::stop()
{
    // Stop the timer
    mStarted = false;

    // Unpause the timer
    mPaused = false;

    // Clear tick variables
    mStartTicks = 0;
    mPausedTicks = 0;
}

void Timer::pause()
{
    // If the timer is running and isn't already paused
    if (mStarted && !mPaused)
    {
        // Pause the timer
        mPaused = true;

        // Calculate the paused ticks
        mPausedTicks = SDL_GetTicks() - mStartTicks;
        mStartTicks = 0;
    }
}

void Timer::unpause()
{
    // If the timer is running and paused
    if (mStarted && mPaused)
    {
        // Unpause the timer
        mPaused = false;

        // Reset the starting ticks
        mStartTicks = SDL_GetTicks() - mPausedTicks;

        // Reset the paused ticks
        mPausedTicks = 0;
    }
}

uint32_t Timer::getTicks()
{
    // The actual timer time
    uint32_t time = 0;

    // If the timer is running
    if (mStarted)
    {
        // If the timer is paused
        if (mPaused)
        {
            // Return the number of ticks when the timer was paused
            time = mPausedTicks;
        }
        else
        {
            // Return the current time minus the start time
            time = SDL_GetTicks() - mStartTicks;
        }
    }

    return time;
}

bool Timer::isStarted()
{
    // Timer is running and paused or unpaused
    return mStarted;
}

bool Timer::isPaused()
{
    // Timer is running and paused
    return mPaused && mStarted;
}
```

Esta clase abastrae la función de SDL `SDL_GetTicks()` y la maneja internamente además de proveer de métodos para manejar el temporizador.

La idea para calcular los FPS es la siguiente, antes de empezar el bucle de juego creamos un temporizador y lo iniciamos:

```cpp
Timer fpsTimer;
fpsTimer.start();
```

También definiremos una variable para contar los fotogramas actuales:

```cpp
int countedFrames = 0;
```

En cada iteración calcularemos los fps de media a partir del valor actual del contador entre el temporizador entre 1000, luego incrementaremos los fotogramas. Estos fps de media `avgFPS` los tendremos como una variable global para renderizar ese valor en el texto de antes:

```cpp
float avgFPS = 0;  // <--- global
//...

avgFPS = countedFrames / (fpsTimer.getTicks() / 1000.f);
++countedFrames
```

Para dibujar los FPS modificaremos el renderizado de texto:

```cpp
text = TTF_RenderText_Solid(font, (std::to_string(avgFPS) + " fps").c_str(), color);
```

Por defecto al tener la sincronización activada los FPS se autoregulan a los hercios de mi pantalla, que son 144:

![](./docs/image-5.png)

Ahora nos falta añadir la limitación de FPS como alternativa a desactivar la sincronización vertical, esto se denomina el *fps cap*. 

Para ello definiremos unas variables globales que manejarán si activamos o no el cap, la limitación de FPS deseada y el cálculo de ticks de pantalla por fotograma que utilizará nuestro futuro `capTimer`:

```cpp
bool enableCap = true;
int fpsCap = 60;
int screenTicksPerFrame = 1000 / fpsCap;
```

Procedemos a definir el `capTimer` antes del `while`:

```cpp
Timer capTimer;
```

Lo iniciaremos justo al comenzar la iteración del `while`:

```cpp
while (is_running){
    capTimer.start();
```

Finalmente, después de renderizar la pantalla al final del `while`, realizaremos un ajuste para retrasar la siguiente iteración (utilizando `SDL_Delay`) en función de los ticks de pantalla que resten para llegar a los `screenTicksPerFrame` calculados antes:

```cpp
// Si el fotograma finaliza demasiado pronto
int frameTicks = capTimer.getTicks();
if (enableCap && frameTicks < screenTicksPerFrame)
{
    // Esperamos el tiempo restante
    SDL_Delay(screenTicksPerFrame - frameTicks);
}
```

Con esto habremos logrado un límite de FPS manual en caso de no querer la sincronización vertical o podemos desactivarlo utilizando `enableCap`:

```cpp
bool enableCap = true;
```

Aquí se aprecia el cap manual a 60FPS:

![](./docs/image-6.png)

### Refactorización 1

Antes de continuar con el siguiente tema sobre vectores y puntos voy a reorganizar los ficheros del proyecto en clases para que todo sea más cómodo de utilizar.

Esencialmente voy a abstraer todo el proceso de gestión de la ventana y renderizado en una clase `Window`.

Las cabeceras `window.h` por ahora quedarán de la siguiente forma, también después de cambiar la notación a **PascalCase** en los métodos y **camelCase** en las variables:

```cpp
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
    float avgFPS = 0;
    bool enableCap = true;
    int fpsCap = 60;
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
    void DrawRect(int sx, int sy, int width, int height, uint32_t color);
};

#endif
```

En cuanto al código fuente `window.cpp` la implementción completa es:

```cpp
#include "window.h"

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
        windowWidth, windowHeight, SDL_WINDOW_BORDERLESS);

    if (!window)
    {
        std::cout << "Error creating SDL Window." << std::endl;
        running = false;
    }

    // Creamos el renderizador SDL
    if (enableCap)
    {
        renderer = SDL_CreateRenderer(window, -1, 0);
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

    // Start Timers
    fpsTimer.start();
    capTimer.start();
}

void Window::ProcessInput()
{
    SDL_Event event;
    SDL_PollEvent(&event);

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
    if (enableCap)
    {
        // Iniciar el temporizador de cap
        capTimer.start();
    }

    // Calculate fps
    avgFPS = countedFrames / (fpsTimer.getTicks() / 1000.f);
    // Increment the frame counter
    ++countedFrames;
}

void Window::Render()
{
    // Limpiar el color buffer
    ClearColorBuffer(static_cast<uint32_t>(0xFF0000000));
}

void Window::PostRender()
{
    // Renderizar el color buffer
    RenderColorBuffer();

    // Render FPS
    textSurface = TTF_RenderText_Solid(textFont, (std::to_string(avgFPS) + " fps").c_str(), textColor);
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

    // Y por último capear los fotogramas si es necesario
    if (enableCap)
    {
        int frameTicks = capTimer.getTicks();
        if (frameTicks < screenTicksPerFrame)
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
```

Inicializar y empezar a trabajar con la ventana es ahora muy sencillo, así quedará `main.cpp`:

```cpp
#include <iostream>
#include "window.h"

int main(int argc, char *argv[])
{
    Window window(640, 480);

    window.Init();
    window.Setup();

    while (window.running)
    {
        window.ProcessInput();

        window.Update();
        window.Render();

        window.DrawGrid(0xFF616161);
        window.DrawRect(50, 50, 100, 100, 0xFF1570E8);
        window.DrawRect(205, 125, 300, 200, 0xFFD93E23);
        window.DrawRect(375, 225, 300, 300, 0xFFE35FDA);

        window.PostRender();
    }

    return 0;
}
```

A comentar la variable pública `window.running` que permite saber en todo momento si la ventana está funcionando para seguir ejecutando el bucle. 

Luego los distintos métodos de mismo nombre `ProcessInput`, `Update`, `Render` y un nuevo `PostRender` que me permite separar el renderizado en dos partes y dibujar entre tanto diferentes elementos y por encima dibujar los FPS y presentar el `renderer`.

Por cierto, tampoco necesitamos limpiar la pantalla en el renderer, pues estamos dibujando directamente nuestro `colorBuffer`:

```cpp
void Window::Render()
{
    // Establecer el color del renderizador
    // SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    // Limpiar la pantalla con el color establecido
    // SDL_RenderClear(renderer);

    // Limpiar el color buffer
    ClearColorBuffer(static_cast<uint32_t>(0xFF0000000));
}
```

## Vectores y puntos

Es hora de empezar a pensar en 3D, eso implica añadir una nueva dimensión: la **profundidad**, que se representa en lo que se conoce como el **eje Z**.

Empezaré creando un nuevo método para pintar un único píxel en pantalla:

```cpp
void Window::DrawPixel(int x, int y, unsigned int color)
{
    if (x >= 0 && x < windowWidth && y >= 0 && y < windowHeight)
    {
        colorBuffer[(windowWidth * y) + x] = static_cast<uint32_t>(color);
    }
}
```

Si queremos representar algo en el espacio tridimensional, debemos hacerlo a través de números que indican cantidades.

Las cantidades se dividen en dos tipos:

* **Cantidades escalares**: Representadas con único número: temperatura, área, longitud, presión...
* **Cantidades vectoriales**: Representadas con más de un número: velocidad, aceleración, fuerza, arrastre, desplazamiento, elevación...

Para representar una cantidad vectorial de dos números, por ejemplo **velocidad (m/s)**, se utiliza un eje de coordenadas 2D y un vector formado por los componentes `X`, `Y`.

De la misma forma podemos representar una cantidad vectorial formada por tres números **ancho**, **alto** y **profundidad** en un eje de coordenadas 3D mediante un vector formado por los componentes `X`, `Y`, `Z`. 

Un **vector** es un conjunto de componentes donde el orden importa para representar una cantidad formada por dos o más números, así que vamos a empezar por definir unas clases para manejar nuestros propios vectores 2D y 3D:

```cpp
#ifndef VECTOR_H
#define VECTOR_H

#include <iostream>

class Vector2
{
public:
    double x;
    double y;

    friend std::ostream &operator<<(std::ostream &os, const Vector2 &v);
};

class Vector3
{
public:
    double x;
    double y;
    double z;

    friend std::ostream &operator<<(std::ostream &os, const Vector3 &v);
};

#endif
```

Para la implementación por ahora solo la sobrecarga del `ostream` para imprimir un vector:

```cpp
#include "vector.h"

std::ostream &operator<<(std::ostream &os, const Vector2 &v)
{
    os << "(" << v.x << ", " << v.y << ")";
    return os;
}

std::ostream &operator<<(std::ostream &os, const Vector3 &v)
{
    os << "(" << v.x << ", " << v.y << ", " << v.z << ")";
    return os;
}
```

Vamos a suponer que deseamos representar un cubo 3D de 9 píxeles de ancho, alto y profundidad. Podemos entenderlo como un conjunto de 9*9*9 píxeles, donde cada punto es un vector tridimensional.

Lo que podemos hacer es suponer el centro del cubo como el origen del eje de coordenadas (0,0,0) y a partir de ahí uniformemente (en base de -1 a 1) representarlo a la izquierda, derecha, arriba, abajo, adelante y atrás.

Para ello lo podemos inicializar con tres bucles anidados:

```cpp
// Cubo de 9x9x9 píxeles
int counterPoints;
Vector3 cubePoints[9 * 9 * 9];

// Cargar el array de vectores de -1 a 1 en el cubo 9x9x9
for (double x = -1; x <= 1; x += 0.25)
{
    for (double y = -1; y <= 1; y += 0.25)
    {
        for (double z = -1; z <= 1; z += 0.25)
        {
            cubePoints[counterPoints++] = Vector3{x, y, z};
        }
    }
}
```

Este es el principio y final del arreglo 9*9*9 con 729 puntos:

```
(-1, -1, -1), 
(-1, -1, -0.75), 
..., 
(0, 0, -0.25), 
(0, 0, 0), 
(0, 0, 0.25), 
..., 
(1, 1, 0.75), 
(1, 1, 1)
```

El problema es que nuestro `ColorBuffer` se fundamenta en un eje 2D de ancho y alto. ¿Cómo podemos a representar un arreglo de vectores 3D en un buffer 2D?

## Proyección de puntos

Las técnicas de proyección nos permiten representar, mediante operaciones matemáticas una dimensión 3D en forma  2D.

Existen varios tipos de proyecciones dependiendo del resultado que nos interese.

### Proyección ortográfica

Esta proyección es una proyección paralela que consiste en ignorar la profundidad (eje `Z`).

Para implementar esta proyección en una función recibiremos un vector 3D y devolveremos un vector 2D con únicamente sus componentes X e Y:

```cpp
Vector2 OrtoraphicProjection(Vector3 p)
{
    return Vector2{p.x, p.y};
}
```

Para probar la función vamos a crear un cubo de puntos proyectos en 2D. Podemos definir el arreglo fuera del `while`:

```cpp
Vector2 cubeProjectedPoints[9 * 9 * 9];
```

Entre `Update` y `Render` proyectamos los puntos a 2D:

```cpp
window.Update();

// Vector 3D proyectado ortográficamente
for (int i = 0; i < 9 * 9 * 9; i++)
{
    // Proyeccion del punto
    cubeProjectedPoints[i] = OrtoraphicProjection(cubePoints[i]);
}

window.Render();
```

Ahora durante el renderizado, podemos hacer uso de nuestro método `DrawPixel` y establecer todos los píxeles del cubo proyectado en el `ColorBuffer`:

```cpp
window.Render();
window.DrawGrid(0xFF616161);

/* Dibujar proyección */
for (int i = 0; i < 9 * 9 * 9; i++)
{
    window.DrawPixel(
        cubeProjectedPoints[i].x,
        cubeProjectedPoints[i].y,
        0xFF00FFFF);
}

window.PostRender();
```

El resultado será el siguiente:

![](./docs/image-7.png)

Un pequeño píxel en la parte superior izquierda. 

¿Por qué? Pues debido a que los valores de nuestro cubo se encuentran normalizados entre `-1` y `1` con el origen en `0`.

Esto nos lleva a la idea de que debemos escalar de alguna forma los valores del cubo.

Este escalar se denomina**FOV** (campo de visión) y podemos probar alguna cantidad hasta dar con la que nos guste y mulitiplicarla en nuestra función de proyección:

```cpp
float fovFactor = 100;

Vector2 OrtoraphicProjection(Vector3 p)
{
    return Vector2{
        fovFactor * p.x, 
        fovFactor * p.y};
}
```

El resultado por ahora será algo así:

![](./docs/image-8.png)

Debemos tener en cuenta que como consecuencia de aplicar el `fovFactor`, el cubo crece en tamaño y para dibujarlo completamente necesitamos más espacio. Por eso deberemos reposicionarlo, idealmente hacia el centro de la pantalla, tomando su origen `(0, 0)` como el punto `(windowWidth/2, windowHeight/2)`.

Así que simplemente sumamos esa distancia en sus componentes durante el renderizado:

```cpp
/* Dibujar proyección reposicionada al centro */
for (int i = 0; i < 9 * 9 * 9; i++)
{
    window.DrawPixel(
        cubeProjectedPoints[i].x + window.windowWidth / 2,
        cubeProjectedPoints[i].y + window.windowHeight / 2,
        0xFF00FFFF);
}
```

Y ya está, ahora sí con su aspecto real en paralelo:

![](./docs/image-9.png)

### Proyección perspectiva

La proyección en perspectiva consiste en simular la forma en cómo los humanos vemos el mundo, donde los objetos cerca nuestro se perciben mayores que los que están lejos.

Esto introduce la idea de que necesitamos una especie de espectador u ojo como origen de la vista tridimensional con un ángulo de visión que definirá el campo visible, llamado `AOV` (angle of view).

En un videojuego o simulación tridimensional, el origen de la vista es la cámara que nos permite percibir el mundo, abarcando el espacio entre el plano más cercano y el plano más alejado, denominado `View Frustrum`:

![](./docs/image-10.png)

![](./docs/image-12.png)

![](./docs/image-11.png)

Mediante el uso de la geometría y la propiedad de los triángulos similares de compartir proporciones equivalentes, podemos calcular las fórmulas para los puntos proyectados `P'x` y `P'y`:

![](https://latex.codecogs.com/png.image?\dpi{150}\bg{white}\frac{P%27x}{Px}=\frac{1}{Pz}\to\frac{Px}{Pz})

![](https://latex.codecogs.com/png.image?\dpi{150}\bg{white}\frac{P%27y}{Py}=\frac{1}{Pz}\to\frac{Py}{Pz})

Ambas fórmulas se conocen como **brechas de perspectiva**, en inglés *perspective divide* y dictan que:

* Cuanto menor sea la profundidad `z`, mayor serán `x` e `y`, de manera que los objetos se percibirán más grandes.
* Cuanto mayor sea la profundidad `z`, menores serán `x` e `y`, de manera que  los objetos se percibirán más pequeños.

Nuestra nueva función de perspectiva simplemente dividirá `x` e `y` entre `z`:

```cpp
Vector2 PerspectiveProjection(Vector3 p)
{
    return Vector2{
        (fovFactor * p.x) / p.z, 
        (fovFactor * p.y) / p.z};
}

cubeProjectedPoints[i] = PerspectiveProjection(cubePoints[i]);
```

EL resultado se verá más o menos así:

![](./docs/image-13.png)

No es exactamente lo que se espera pero se percibe una especie de profundidad. 

La razón por la que se ve de esta forma es que estamos suponiendo que el ojo, el origen de la vista, concuerda justo en la cara más profunda del cubo.

Para solucionarlo debemos alejar nuestra vista del cubo, esto lo conseguiremos restando una profundidad extra mediante un `Vector3` para simular la posición de una cámara alejada del fondo del cubo:

```cpp
Vector3 cameraPosition{0, 0, -5};
```

Esta distancia la vamos a restar del punto antes de realizar la proyección de perspectiva:

```cpp
window.Update();

for (int i = 0; i < 9 * 9 * 9; i++)
{
    // Restamos la distancia de la cámara
    Vector3 point = cubePoints[i];
    point.z -= cameraPosition.z;
    // Proyeccion del punto
    cubeProjectedPoints[i] = PerspectiveProjection(point);
}

window.Render();
```

Si visualizamos el cubo lo visualizaremos muy pequeño pero ya se podrá apreciar la perspectiva:

![](./docs/image-14.png)

Podemos rectificar el tamaño mediante la profundidad de la cámara `cameraPosition.z` o con el factor de escalado del punto de vista `fovFactor`, probemos cambiando éste último:

```cpp
float fovFactor = 200;
```

Al aumentar el factor de escalado el cubo se percibe más grande:

![](./docs/image-15.png)

Estos valores no son casuales, todo esto tiene una explicación matemática clara.

Dado que el lado del cubo mide 2 unidades uniformes (de -1 a 1), un factor de 200 ocasionará que el cubo tenga un tamaño de -200 a 200 píxeles al escalarlo, por lo que lado completo medirá 400px.

Ahora bien, como la cámara está a 5 unidades de distancia, podemos suponer que el tamaño que percibiremos será 400/5 = 80px... ¿O no? Pues no, el tamaño del costado es exactamente 100px:

![](./docs/image-16.png)

¿Recordáis que al dibujar el cubo lo hacemos desde su cara más profunda?

Considerando eso debemos suponer que para dibujar su cara más cercana debemos alejarnos de la cara profunda exactamente lo que mide el costado del cubo, es decir 2 unidades (200 * 2 px):

```cpp
Vector3 cameraPosition{0, 0, -2};
```

Si nuestra suposición es correcta, desde esta posición de la cámara el costado tendrá un tamaño exacto de 400px:

![](./docs/image-17.png)

### Regla de la mano

En nuestro entorno tridimensional hemos asumido algo importante sin darnos cuenta, me refiero a la dirección de crecimiento para la profundidad en el eje `Z`.

Hemos considerado que cuanto mayor sea la `Z` más profundidad y cuanto menor sea, menos profundidad. Precisamente por eso le restamos al eje `Z` de la cámara `(0,0,-5)`, para alejarla del cubo.

Sin embargo sistemas como **OpenGL** se basan en lo contrario, cuanto mayor sea la `Z` menos profundidad y cuanto menor sea, más profundidad. En ese sistema  para alejar la cámara deberíamos sumar `(0,0,5)` al eje `Z` :

![](./docs/image-18.png)

La dirección de la profundidad es un tema importante en la programación gráfica, la forma de realizar algunos cálculos es distinta dependiendo del sistema elegido.

Si ponemos el pulgar de la mano derecha  mirando hacia la derecha simulando el eje `X` y el índice hacia arriba simulando el eje `Y`, el dedo corazón apuntará hacia nosotros, diremos que la profundidad `Z` crece hacia fuera de la pantalla. Pero si repetimos el proceso con la mano izquierda, el dedo corazón apuntará al lado inverso, la profundidad `Z` crece hacia la pantalla:

![](./docs/image-19.png)

Esto se conoce como **regla de la mano** y nos permite determinar sentidos vectoriales. Nuestro sistema, al igual que **DirectX**, se basa en la mano izquierda (la profundidad crece hacia adentro de la pantalla), mientras que **OpenGL** se basa en la mano derecha, (la profundidad crece hacia afuera de la pantalla). 

Recordar esta sencilla regla nos servirá para más adelante.

## Transformaciones lineales

Hemos visto como dividiendo entre el eje `Z` hemos conseguido la **brecha de perspectiva** para la proyección en perspectiva. Sin embargo, esta función es solo uno de los diferentes pasos que necesitamos para conseguir la verdadera proyección en perspectiva.

A parte de dvidir entre `Z` necesitamos considerar por ejemplo, cuál es ángulo del `FOV` (el campo de visión) que estamos utilizando, o también el `AR` (la relación de aspecto) de la pantalla. Para conseguir esto necesitaremos acudir a la **proyección de matrices**, pero es un tema que trataremos más adelante.

Por ahora utilizaremos la **brecha de perspectiva** y nos centraremos en algo más interesante, añadir dinamismo a nuestro cubo mediante la aplicación de transformaciones en sus vectores.

### Transformando vectores

Para transformar los vectores necesitamos acudir al **álgebra lineal**, la rama de la matemática que estudia las ecuaciones y funciones lineales, así cómo su representación como vectores y matrices.

Las tres transformaciones esenciales son:

* **Escalado**: Para hacer más grande o más pequeño el vector.
* **Traslación**: Para mover el vector de sitio a otro.
* **Rotación**: Para rotar el vector una cierta cantidad.

Las transformaciones las llevaremos a cabo antes de la proyección, durante el evento de actualización y antes del renderizado:

1. `ProcessInput()`
2. `Update()`
    1. `TransformPoints()`
        1. `Rotate(x, y, z)`
        2. `Scale(amout)`
        3. `Translate(amout)`
    2. `ProjectPoints()`
3. `Render()`

### Razones trigonométricas

Para realizar las transformaciones de los vectores es necesario utilizar la trigonometría, así que vamos a repasar los conceptos básicos.

Trigonometría significa **estudio de los trígonos**, polígonos con tres lados y tres ángulos.

Las razones trigonométricas son las relaciones entre los lados de un triángulo rectángulo:

![](./docs/image-20.png)

![](https://latex.codecogs.com/png.image?\dpi{150}\bg{white}sin(\alpha)&space;=&space;\frac{opuesto}{hipotenusa}&space;\rightarrow&space;s&space;=&space;o&space;/&space;h)

![](https://latex.codecogs.com/png.image?\dpi{150}\bg{white}cos(\alpha)&space;=&space;\frac{adyacente}{hipotenusa}&space;\rightarrow&space;c&space;=&space;a&space;/&space;h)

![](https://latex.codecogs.com/png.image?\dpi{150}\bg{white}tan(\alpha)&space;=&space;\frac{opuesto}{adyacente}&space;\rightarrow&space;t&space;=&space;o&space;/&space;a)

Estas relaciones nos permitirán realizar distintos cálculos esenciales para las transformaciones lineales como por ejemplo la rotación. 

Una forma de recordarlas es mediante la palabra **sohcahtoa**.

### Rotación de vectores

Ahora que hemos repasado el tema de las razones trignométricas podemos aplicar esos conceptos para rotar los vectores de nuestro cubo.

Para rotar un vector (x,y) se debe aplicar la fórmula de la rotación de matrices:

<img src="https://latex.codecogs.com/png.image?\dpi{150}\bg{white}\begin{bmatrix}cos&space;(\alpha)&space;&&space;-sin(\alpha)\\sin(\alpha)&space;&&space;cos(\alpha)\\\end{bmatrix}&space;*\begin{bmatrix}x&space;\\y&space;\\\end{bmatrix}"/>

Dado un vector `(x, y)` queremos aplicarle una rotación (ángulo) para conseguir el nuevo vector rotado `(x', y')`.

![](./docs/image-21.png)

Tomando el triángulo que forma `(x, y)` con ángulo `β` y el triángulo que forma `(x', y')` con ángulo `θ` podemos establecer sus relaciones trigonométricas para encontrar los valores del vector rotado `(x', y')`:

![](./docs/image-22.png)

Para determinar los valores rotados necesitamos asumimos el ángulo como la suma de `θ` y `β`: 

<img src="https://latex.codecogs.com/png.image?\dpi{150}\bg{white}\\cos(&space;\theta&space;)&space;=&space;x/r&space;\to&space;x&space;=&space;r&space;*&space;cos(\theta)\\sin(&space;\theta&space;)&space;=&space;y/r&space;\to&space;y&space;=&space;r&space;*&space;sin(\theta)\\\\cos(&space;\theta&space;&space;&plus;&space;\beta&space;)&space;=&space;x'/r&space;\to&space;x'&space;=&space;r'&space;*&space;cos(\theta&space;&plus;&space;\beta)\\sin(&space;\theta&space;&space;&plus;&space;\beta)&space;=&space;y'/r&space;\to&space;y'&space;=&space;r'&space;*&space;sin(\theta&space;&plus;&space;\beta)"/>

No sabemos el ángulo `β` pero en trigonometría existen las conocidas **fórmulas de adición** que explican como expandir el seno y el coseno de una suma o resta de dos ángulos:

![](./docs/image-24.png)

Aplicando la fórmula de adición del ángulo para el coseno:

<img src="https://latex.codecogs.com/png.image?\dpi{150}\bg{white}\\x'&space;=&space;rcos(\theta&space;&plus;&space;\beta)\\x'&space;=&space;r(cos\theta&space;cos&space;\beta&space;-&space;sin\theta&space;sin\beta)\\x'&space;=&space;rcos\theta&space;cos&space;\beta&space;-&space;rsin\theta&space;sin\beta"/>

Aquí podemos substituir `rcosθ` por `x`y `rsinθ` por `y` para conseguir finalmente el valor de `x'`:

<img src="https://latex.codecogs.com/png.image?\dpi{150}\bg{white}x'&space;=&space;x&space;cos&space;\beta&space;-&space;y&space;sin\beta"/>

Lo mismo podemos formular para conseguir el valor de `y'`:

<img src="https://latex.codecogs.com/png.image?\dpi{150}\bg{white}\\&space;y'&space;=&space;rsin(\theta&space;&plus;&space;\beta)\\&space;y'&space;=&space;r(sin\theta&space;cos&space;\beta&space;&plus;&space;cos&space;\theta&space;sin&space;\beta)\\&space;y'&space;=&space;rsin&space;\theta&space;cos&space;\beta&space;&plus;&space;rcos&space;\theta&space;sin&space;\beta\\&space;"/>

Substituimos `rsinθ` por `y` y `rcosθ` por `x` para conseguir el valor de `y'`:

<img src="https://latex.codecogs.com/png.image?\dpi{150}\bg{white}\\&space;y'&space;=&space;y&space;cos\beta&space;&plus;&space;x&space;sin&space;\beta&space;"/>

Y ya tenemos las ecuaciones que podemos escribir en forma matricial para un ángulo `α`:

<img src="https://latex.codecogs.com/png.image?\dpi{150}\bg{white}\begin{bmatrix}cos&space;(\alpha)&space;&&space;-sin(\alpha)\\sin(\alpha)&space;&&space;cos(\alpha)\\\end{bmatrix}&space;*\begin{bmatrix}x&space;\\y&space;\\\end{bmatrix}"/>

Ahora bien, esto es para rotar un vector 2D, ¿cómo se aplica con un vector 3D?

Pues si nos lo paramos a pensar, podemos rotar un elemento en base a cualquier de los dimensiones. No es lo mismo rotar algo horizontalmente, que verticalmente o en profundidad.

Al tener tres tipos de rotación el eje aldedor del que vamos a rotar quedará congelado, es decir, no se modificará y es precisamente gracias a eso que no necesitamos nada más para manejar las rotaciones.

Para rotar alrededor del eje `z`:

<img src="https://latex.codecogs.com/png.image?\dpi{150}\bg{white}\\&space;x'&space;=&space;xcos&space;\beta&space;-&space;y&space;sin&space;\beta\\&space;y'&space;=&space;xsin\beta&space;&plus;&space;ycos\beta\\&space;z'&space;=&space;inmutable"/>

Para rotar alrededor del eje `y`:

<img src="https://latex.codecogs.com/png.image?\dpi{150}\bg{white}\\&space;x'&space;=&space;xcos&space;\beta&space;-&space;z&space;sin&space;\beta\\&space;y'&space;=&space;inmutable\\&space;z'&space;=&space;xsin\beta&plus;zcos\beta"/>

Para rotar alrededor del eje `x`:

<img src="https://latex.codecogs.com/png.image?\dpi{150}\bg{white}\\&space;x'&space;=&space;inmutable\\&space;y'&space;=&space;ycos&space;\beta&space;-&space;z&space;sin&space;\beta\\&space;z'&space;=&space;ysin\beta&plus;zcos\beta"/>

### Función de rotación

Después de tanta trigonometría es hora de programar un poco y llevar a código todo lo que hemos aprendido.

Empecemos creando unos métodos para rotar los vectores 3D:

```cpp
class Vector3
{
public:
    double x;
    double y;
    double z;

    friend std::ostream &operator<<(std::ostream &os, const Vector3 &v);

    void RotateX(float angle);
    void RotateY(float angle);
    void RotateZ(float angle);
};
```

La implementación siguiendo las fórmulas de la lección anterior:

```cpp
void Vector3::RotateX(float angle)
{
    double newY = y * cos(angle) - z * sin(angle);
    double newZ = y * sin(angle) + z * cos(angle);

    y = newY;
    z = newZ;
}
void Vector3::RotateY(float angle)
{
    double newX = x * cos(angle) - z * sin(angle);
    double newZ = x * sin(angle) + z * cos(angle);

    x = newX;
    z = newZ;
}
void Vector3::RotateZ(float angle)
{
    double newX = x * cos(angle) - y * sin(angle);
    double newY = x * sin(angle) + y * cos(angle);

    x = newX;
    y = newY;
}
```

Ahora, podemos aplicar la rotación a partir de un `Vector3 cubeRotation` declarado fuera del `while`:

```cpp
// Rotación cubo
Vector3 cubeRotation;
```

Justo antes de proyectar el cubo a 2D, le aplicaremos las transformaciones:

```cpp
window.Update();

// Rotation transformations per frame
cubeRotation.x += 0.01;
cubeRotation.y += 0.01;
cubeRotation.z += 0.01;

for (size_t i = 0; i < 9 * 9 * 9; i++)
{
    Vector3 point = cubePoints[i];
    // Rotation transformations
    point.RotateX(cubeRotation.x);
    point.RotateY(cubeRotation.y);
    point.RotateZ(cubeRotation.z);
    // Restamos la distancia de la cámara
    point.z -= cameraPosition.z;
    // Proyeccion del punto
    cubeProjectedPoints[i] = PerspectiveProjection(point);
}

window.Render();
```

Como resultado tendremos el cubo rotando:

![](./docs/anim-01.gif)

### Refactorización 2

Tenemos mucho que refactorizar en la ventana y nuestros vectores, así que pongámonos manos a la obra.

Empezando por las variables del `fovFactor` y el `cameraPosition` dentro de la ventana como variables públicas, eso nos permitirá luego, a través de un puntero de `window`, acceder desde cualquier lugar a ellos.

```cpp
#include "vector.h"

class Window
{
public:
    float fovFactor = 400;
    Vector3 cameraPosition{0, 0, -5};
```

A continuación los métodos de proyección podemos incluirlos en nuestros vectores de manera que un `Vector3` devuelva ya su versión `Vector2` proyectada a partir de un `fovFactor`:

```cpp
class Vector3
{
public:
    Vector2 OrtoraphicProjection(float fovFactor);
    Vector2 PerspectiveProjection(float fovFactor);
};
```

La implementación quedaría así:

```cpp
Vector2 Vector3::OrtoraphicProjection(float fovFactor)
{
    return Vector2{fovFactor * x, fovFactor * y};
}

Vector2 Vector3::PerspectiveProjection(float fovFactor)
{
    return Vector2{(fovFactor * x) / z, (fovFactor * y) / z};
}
```

Para utilizarlos simplemente cambiaremos:

```cpp
// cubeProjectedPoints[i] = PerspectiveProjection(point);
cubeProjectedPoints[i] = point.PerspectiveProjection(fovFactor);
```

También refactorizaremos los tres métodos de rotar X, Y, Z en un único método que tome un `Vector3` para las cantidades y el ángulo:

```cpp
void Rotate(Vector3 angles);
```

La implementación se ejecutará en función de los valores del Vector de cantidades

```cpp
void Vector3::Rotate(Vector3 angles)
{
    if (angles.x != 0)
        RotateX(angles.x);
    if (angles.y != 0)
        RotateY(angles.y);
    if (angles.z != 0)
        RotateZ(angles.z);
}
```

Llamarlo será tan sencillo como:

```cpp
// Rotation transformations
point.Rotate(cubeRotation);
```

En este punto lo más interesante sería crear nuestra propia clase `cube` para crear y renderizar cubos de forma sencilla, además nos permitirá separar cómodamente la función principal de los objetos tridimensionales:

```cpp
#ifndef CUBE_H
#define CUBE_H

#include <iostream>
#include <memory>
#include "vector.h"

// Para prevenir dependencias cíclicas
class Window;

class Cube
{
public:
    size_t pointsCounter{0};
    std::unique_ptr<Vector2[]> projectedPoints;

private:
    Window *window;
    Vector3 rotation;
    Vector3 rotationAmount;
    std::unique_ptr<Vector3[]> points;

public:
    Cube() = default;
    Cube(Window *window, int length);
    void SetRotationAmount(float x, float y, float z);
    void Update();
    void Render();
};

#endif
```

La implementación factorizada con el puntero de ventana y algunas mejores quedará de esta forma:

```cpp
#include "cube.h"
#include "window.h" // Importamos la fuente de la ventana

Cube::Cube(Window *window, int pointsPerSide)
{
    this->window = window;

    // Si el numero de puntos por lado es par le sumamos 1
    // El centro del cuadrado es el punto intermedio 0,0,0
    // Por eso necesitamos asegurarnos de poder dividirlo
    if (pointsPerSide % 2 == 0)
        pointsPerSide++;

    points = std::make_unique<Vector3[]>(pointsPerSide * pointsPerSide * pointsPerSide);
    projectedPoints = std::make_unique<Vector2[]>(pointsPerSide * pointsPerSide * pointsPerSide);

    // Array de vectores de -1 a 1 (requiere longitud impar)
    float portion = 1.0f / (pointsPerSide / 2);

    for (float x = -1.0; x <= 1; x += portion)
    {
        for (float y = -1.0; y <= 1; y += portion)
        {
            for (float z = -1.0; z <= 1; z += portion)
            {
                // std::cout << x << "," << y << "," << z << std::endl;
                points[pointsCounter++] = Vector3{x, y, z};
            }
        }
    }
}

void Cube::Update()
{
    // Set new framr rotation amounts
    rotation.x += rotationAmount.x;
    rotation.y += rotationAmount.y;
    rotation.z += rotationAmount.x;

    for (size_t i = 0; i < pointsCounter; i++)
    {
        Vector3 point = points[i];
        // Rotation transformation
        point.Rotate(rotation);
        //  Restamos la distancia de la cámara
        point.z -= window->cameraPosition.z;
        // Proyeccion del punto
        projectedPoints[i] = point.PerspectiveProjection(window->fovFactor);
    }
}

void Cube::SetRotationAmount(float x, float y, float z)
{
    rotationAmount = {.x = x, .y = y, .z = z};
}

void Cube::Render()
{
    /* Dibujar proyección reposicionada al centro */
    for (size_t i = 0; i < pointsCounter; i++)
    {
        window->DrawPixel(
            projectedPoints[i].x + window->windowWidth / 2,
            projectedPoints[i].y + window->windowHeight / 2,
            0xFF00FFFF);
    }
}
```

Podemos pasar la referencia de la ventana al cubo, que lo crearemos en una variable privada `cube`:

```cpp
class Window
{
private:
    /* Custom objects */
    Cube cube;
```

Lo inicializaremos en el método `window.Setup()`, así como su rotación:

```cpp
#include "cube.h"

void Window::Setup()
{
    // Custom objects
    cube = Cube(this, 7);
    cube.SetRotationAmount(0.01, 0.01, 0.01);
}
```

Realizamos los cálculos de transformación y proyección mediante en el `window.Update()`:

```cpp
void Window::Update()
{
    // Custom objects updating
    cube.Update();
}
```

Y lo renderizaremos en el `window.Render()`:

```cpp
// Custom objects rendering
cube.Render();
```

En este punto podemos eliminar `window.PostRender()` del bucle `while` y establecer la llamada al final del propio `Render` ya que a partir de ahora los elementos los manejaremos desde la ventana:

```cpp
void Window::Render()
{
    // Clear color buffer
    ClearColorBuffer(static_cast<uint32_t>(0xFF0000000));

    // Render the background grid
    DrawGrid(0xFF616161);

    // Custom objects renderring
    cube.Render();

    // Late rendering actions
    PostRender();
}
```

El resultado final será el mismo de antes:

![](./docs/anim-02.gif)

Pero al refactorizar el código la función `main` quedará super limpia:

```cpp
#include <iostream>
#include "window.h"

int main(int argc, char *argv[])
{
    Window window(640, 480);

    window.Init();
    window.Setup();

    while (window.running)
    {
        window.ProcessInput();
        window.Update();
        window.Render();
    }

    return 0;
}
```

Además en el futuro, una vez implementemos la transformación de traslación, podremos dibujar múltiples cubos con sus propiedades independientes, tiempo al tiempo.

## Triángulos, vértices y mallas

EL renderizado de objetos tridimensionales se basa, generalmente, en **mallas** (*meshes*). 

Una malla es una colección de triángulos organizados en el espacio 3D para crear la impresión de un objeto sólido:

![](./docs/image-25.png)

Cada uno de los triángulos, llamados caras o *faces* se define por tres coordenadas de esquinas llamadas comúnmente **vértices**, representables mediante vectores:

![](./docs/image-26.png)

Para nuestra aplicación, un cubo está formado por una malla con 12 caras, pero como algunas comparten vértices, solo necesitamos 8 vértices para representar todas las caras:

![](./docs/image-27.png)

Como nuestra idea es una representación de -1 a 1 uniforme, el arreglo de vértices para un cubo podría ser este:

```cpp
Vector3 vertices[8] = {
    {-1, -1, -1},  // 0
    { 1, -1, -1},  // 1
    {-1,  1, -1},  // 2
    { 1,  1, -1},  // 3
    {-1, -1,  1},  // 4
    { 1, -1,  1},  // 5
    {-1,  1,  1},  // 6
    { 1,  1,  1}   // 7
}
```

Ahora podemos crear un nuevo arreglo definiendo las caras del cubo haciendo referencia a los índices del arreglo de vértices:

```cpp
Vector3 faces[12] = {
    // front    ->
    {1, 0, 2},
    {1, 2, 3},
    // back     <-
    {4, 5, 7},
    {4, 7, 6},
    // right    ->
    {1, 7, 5},
    {1, 3, 7},
    // left     <-
    {4, 2, 0}, 
    {4, 6, 2},
    // top      ->
    {2, 7, 3}, 
    {2, 6, 7} 
    // bottom   <- 
    {1, 5, 4}, 
    {1, 4, 0} 
}
```

El orden de los vértices en las caras es muy importante porque determina la profundidad, la dirección hacia donde mira la cara.

Una cara establecida en sentido horario, como `(0, 3, 1)` apunta hacia **adelante**, acercándose a nosotros. Pero si se establece en sentido antihorario `(0, 1, 3)` apunta hacia **atrás**, alejándose de nosotros.

Las caras deben mirar acordemente hacia donde están siendo dibujadas. La cara delantera hacia adelante (sentido horario), la caras trasera hacia atrás (sentido antihorario). 

Esto se puede apreciar en la siguiente figura, donde de hecho las caras que no nos ven nunca se llegan a dibujar:

![](./docs/image-28.png)

En el caso de nuestro cubo, la cara delantera, superior y derecha miran en sentido horario, mientras que la trasera, inferior e izquierda miran hacia en  sentido antiohorario. 

### Programando los triángulos

Así que vamos a ponernos manos a la obra, empezando por crear una nueva clase llama `Triangle` en `triangle.h` que almacenará los tres vertices del triángulo tanto en 3D como en 2D una vez estén ya proyectados, algo que haremos llamando la función `ProjectVertex` pasándole el `vertexIndex` y el `fovFactor`. También he abstraido implementaciones para las transformaciones de rotar un vértice `RotateVertex` y moverlo `TranslateVertex`:

```cpp
#ifndef TRIANGLE_H
#define TRIANGLE_H

#include "vector.h"

class Triangle
{
public:
    Vector3 vertices[3];
    Vector2 projectedVertices[3];

    void ProjectVertex(int vertexIndex, float fovFactor)
    {
        projectedVertices[vertexIndex] = vertices[vertexIndex].PerspectiveProjection(fovFactor);
    };

    void RotateVertex(int vertexIndex, Vector3 rotation)
    {
        vertices[vertexIndex].Rotate(rotation);
    }

    void TranslateVertex(int vertexIndex, Vector3 distance)
    {
        vertices[vertexIndex].x -= distance.x;
        vertices[vertexIndex].y -= distance.y;
        vertices[vertexIndex].z -= distance.z;
    }
};

#endif
```

Dentro de la cabecera `cube.h` vamos a definir tres arreglos privados para gestionar los vértices, las caras y los triángulos proyectados, justo como expliqué anteriormente.

Aprovecharé para refactorizar y borrar código que ya no nos interesa:

```cpp
#ifndef CUBE_H
#define CUBE_H

#include <iostream>
#include <memory>
#include "vector.h"
#include "triangle.h"

// Para prevenir dependencias cíclicas
class Window;

class Cube
{
private:
    Window *window;
    Vector3 rotation;
    Vector3 rotationAmount;
    std::unique_ptr<Vector3[]> points;
    Vector3 meshVertices[8] = {
        {-1, -1, -1}, // 0
        {1, -1, -1},  // 1
        {-1, 1, -1},  // 2
        {1, 1, -1},   // 3
        {-1, -1, 1},  // 4
        {1, -1, 1},   // 5
        {-1, 1, 1},   // 6
        {1, 1, 1}     // 7
    };
    Vector3 meshFaces[12] = {
        // front    ->
        {1, 0, 2},
        {1, 2, 3},
        // back     <-
        {4, 5, 7},
        {4, 7, 6},
        // right    ->
        {1, 7, 5},
        {1, 3, 7},
        // left     <-
        {4, 2, 0},
        {4, 6, 2},
        // top      ->
        {2, 7, 3},
        {2, 6, 7},
        // bottom   <-
        {1, 5, 4},
        {1, 4, 0} //
    };
    Triangle trianglesToRender[12];

public:
    Cube() = default;
    Cube(Window *window);
    void SetRotationAmount(float x, float y, float z);
    void Update();
    void Render();
};

#endif
```

Durante el evento `cube.Update()` vamos a iterar las caras de todos los triángulos, generando un nuevo `Triangle` a partir de `meshVertices` usando los índices de `meshFaces`. 

A los vértices de ese triángulo `Triangle.vertices` les aplicaremos las transformaciones, rotación, traslación de la cámara y luego proyección a 2D, que almacenaremos en `Triangle.projectedVertices` y rectificación de la posición en la pantalla:

```cpp
void Cube::Update()
{
    // Set new framr rotation amounts
    rotation.x += rotationAmount.x;
    rotation.y += rotationAmount.y;
    rotation.z += rotationAmount.x;

    // Loop all triangle faces of the mesh
    for (size_t i = 0; i < 12; i++)
    {
        // Create a new triangle to store data and render it later
        Triangle triangle;
        triangle.vertices[0] = meshVertices[static_cast<int>(meshFaces[i].x)];
        triangle.vertices[1] = meshVertices[static_cast<int>(meshFaces[i].y)];
        triangle.vertices[2] = meshVertices[static_cast<int>(meshFaces[i].z)];

        // Loop all vertice for the face and apply transformations
        for (size_t j = 0; j < 3; j++)
        {
            // rotate and translate vertex array form the camera
            triangle.RotateVertex(j, rotation);
            triangle.TranslateVertex(j, window->cameraPosition);
            // project the vertex and scale it from 3D to 2D
            triangle.ProjectVertex(j, window->fovFactor);
            // translate the projected point to the middle screen
            triangle.projectedVertices[j].x += (window->windowWidth / 2);
            triangle.projectedVertices[j].y += (window->windowHeight / 2);
        }

        // Save the projected triangle in triangles render array
        trianglesToRender[i] = triangle;
    }
}
```

El renderizado es lo más simple del mundo, recorremos los triángulos proyectados y dibujamos cada uno de sus 3 vértices:

```cpp
void Cube::Render()
{

    // Loop projected triangles array and render them
    for (size_t i = 0; i < 12; i++)
    {
        for (size_t j = 0; j < 3; j++)
        {
            window->DrawPixel(
                trianglesToRender[i].projectedVertices[j].x,
                trianglesToRender[i].projectedVertices[j].y,
                0xFF00FFFF);
        }
    }
}
```

El resultado es parecido a lo que teníamos hasta ahora, pero en lugar de tener muchos puntos solo tenemos los vectores de los vértices:

![](./docs/anim-03.gif)

La parte divertida viene a partir de ahora, pues debemos conectar los puntos e incluso pintar las superficies o añadirles texturas.