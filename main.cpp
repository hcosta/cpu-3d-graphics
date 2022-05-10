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