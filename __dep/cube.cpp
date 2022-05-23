#include "cube.h"
#include "window.h" // Importamos la fuente de la ventana

void Cube::SetRotationAmount(float x, float y, float z)
{
    rotationAmount = {x, y, z};
}

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

void Cube::Render()
{
    // Loop projected triangles array and render them
    for (size_t i = 0; i < 12; i++)
    {
        window->DrawTriangle(
            trianglesToRender[i].projectedVertices[0].x,
            trianglesToRender[i].projectedVertices[0].y,
            trianglesToRender[i].projectedVertices[1].x,
            trianglesToRender[i].projectedVertices[1].y,
            trianglesToRender[i].projectedVertices[2].x,
            trianglesToRender[i].projectedVertices[2].y,
            0xFF00FFFF);
    }
}
