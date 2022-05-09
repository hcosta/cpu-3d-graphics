# CPU 3D Graphics

La meta de este proyecto es entender el pipeline de funcionamiento en el renderizado 3D sin utilizar la GPU, implementando en C++ y desde cero todo el sistema.

Se utilizará SDL2 como biblioteca multiplataforma para manejar el hardware del sistema.

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
all:
	g++ -I src/include -L src/lib -o bin/main main.cpp -lmingw32 -lSDL2main -lSDL2
```

La compilación y ejecución es tan sencilla como ejecutar:

```
> make
> .\bin\main.exe
```

Para facilitar la compilación y ejecución utilizo el complemento [Code Runner](https://marketplace.visualstudio.com/items?itemName=formulahendry.code-runner) de VSC y un comando personalizado definido en el `.vscode/settings.json`:

```json
{
    "code-runner.customCommand": "make && ./bin/main.exe"
}
```

Ejecutable con `F1 > Run Custom Command` y voilà:

![](./docs/image-1.png)

## Configuración previa
