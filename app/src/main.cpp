#include <iostream>
#include "window.h"

int main(int argc, char *argv[])
{
    Window window;

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