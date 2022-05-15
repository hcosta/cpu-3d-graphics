#include "mesh.h"
#include "window.h" // Importamos la fuente de la ventana

Mesh::Mesh(Window *window, Vector3 *vertices, int verticesLength, Vector3 *faces, int facesLength)
{
    this->window = window;
    // Initialize the dinamic vertices and faces
    for (size_t i = 0; i < verticesLength; i++)
        this->vertices.push_back(vertices[i]);
    for (size_t i = 0; i < facesLength; i++)
        this->faces.push_back(faces[i]);
};

void Mesh::SetRotationAmount(float x, float y, float z)
{
    rotationAmount = {.x = x, .y = y, .z = z};
}

void Mesh::Update()
{
    // Set new framr rotation amounts
    rotation.x += rotationAmount.x;
    rotation.y += rotationAmount.y;
    rotation.z += rotationAmount.x;

    // Clear the triangles queue
    triangles.clear();

    // Loop all triangle faces of the mesh
    for (size_t i = 0; i < faces.size(); i++)
    {
        // Create a new triangle to store data and render it later
        Triangle triangle;
        triangle.vertices[0] = vertices[static_cast<int>(faces[i].x)];
        triangle.vertices[1] = vertices[static_cast<int>(faces[i].y)];
        triangle.vertices[2] = vertices[static_cast<int>(faces[i].z)];

        // Loop all vertice for the face and apply transformations
        for (size_t j = 0; j < 3; j++)
        {
            // Rotation
            triangle.RotateVertex(j, rotation);
            // Translation (away from camera)
            triangle.TranslateVertex(j, window->cameraPosition);
            // project the vertex and scale it from 3D to 2D
            triangle.ProjectVertex(j, window->fovFactor);
            // Translate the projected vertex to the middle screen
            triangle.projectedVertices[j].x += (window->windowWidth / 2);
            triangle.projectedVertices[j].y += (window->windowHeight / 2);
        }

        // Push transformed triangles
        triangles.push_back(triangle);
    }
}

void Mesh::Render()
{
    // Loop projected triangles array and render them
    for (size_t i = 0; i < triangles.size(); i++)
    {
        window->DrawTriangle(
            triangles[i].projectedVertices[0].x,
            triangles[i].projectedVertices[0].y,
            triangles[i].projectedVertices[1].x,
            triangles[i].projectedVertices[1].y,
            triangles[i].projectedVertices[2].x,
            triangles[i].projectedVertices[2].y,
            0xFF00FFFF);
    }
}