#include <iostream>
#include "window.h"
#include "vector.h"

float fovFactor = 200;
Vector3 cameraPosition{0, 0, -1};

Vector2 OrtoraphicProjection(Vector3 p)
{
    return Vector2{
        fovFactor * p.x,
        fovFactor * p.y};
}

Vector2 PerspectiveProjection(Vector3 p)
{
    return Vector2{
        (fovFactor * p.x) / p.z,
        (fovFactor * p.y) / p.z};
}

int main(int argc, char *argv[])
{
    Window window(640, 480);

    window.Init();
    window.Setup();

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

    // Vector 3D proyectado ortográficamente
    Vector2 cubeProjectedPoints[9 * 9 * 9];
    for (size_t i = 0; i < 9 * 9 * 9; i++)
    {
        // Restamos la distancia de la cámara
        Vector3 point = cubePoints[i];
        point.z -= cameraPosition.z;
        // Proyeccion del punto
        cubeProjectedPoints[i] = PerspectiveProjection(point);
    }

    while (window.running)
    {
        window.ProcessInput();

        window.Update();
        window.Render();

        window.DrawGrid(0xFF616161);

        /* Dibujar proyección reposicionada al centro */
        for (size_t i = 0; i < 9 * 9 * 9; i++)
        {
            window.DrawPixel(
                cubeProjectedPoints[i].x + window.windowWidth / 2,
                cubeProjectedPoints[i].y + window.windowHeight / 2,
                0xFF00FFFF);
        }

        window.PostRender();
    }

    return 0;
}