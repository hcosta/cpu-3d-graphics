#include "mesh.h"
#include "window.h" // Importamos la fuente de la ventana
#include <fstream>
#include <algorithm>
#include "matrix.h"

Mesh::Mesh(Window *window, std::string fileName)
{
    this->window = window;

    // Open the file
    std::ifstream file(fileName);
    if (!file.is_open())
    {
        std::cerr << "Error reading the file " << fileName << std::endl;
        return;
    }
    // If file is loaded in memory read each line
    std::string line;
    while (std::getline(file, line))
    {
        // if starts with v it's a vertex
        if (line.rfind("v ", 0) == 0)
        {
            Vector3 vertex; // %lf -> double (large float)
            sscanf(line.c_str(), "v %lf %lf %lf", &vertex.x, &vertex.y, &vertex.z);
            this->vertices.push_back(vertex);
        }
        // if starts with f it's a face
        else if (line.rfind("f ", 0) == 0)
        {
            int vertexIndices[3];
            int textureIndices[3];
            int normalIndices[3];
            sscanf(line.c_str(), "f %d/%d/%d %d/%d/%d %d/%d/%d",
                   &vertexIndices[0], &textureIndices[0], &normalIndices[0],
                   &vertexIndices[1], &textureIndices[1], &normalIndices[1],
                   &vertexIndices[2], &textureIndices[2], &normalIndices[2]);
            Vector3 face;
            face.x = vertexIndices[0];
            face.y = vertexIndices[1];
            face.z = vertexIndices[2];
            this->faces.push_back(face);
            this->triangles.push_back(Triangle(0xFFFFFFFF));
        }
    }
}

Mesh::Mesh(Window *window, Vector3 *vertices, int verticesLength, Vector3 *faces, int facesLength, uint32_t *colors)
{
    this->window = window;
    // Initialize the dinamic vertices
    for (size_t i = 0; i < verticesLength; i++)
    {
        this->vertices.push_back(vertices[i]);
    }
    // Initialize the dinamic faces and empty triangles (same number)
    for (size_t i = 0; i < facesLength; i++)
    {
        this->faces.push_back(faces[i]);
        this->triangles.push_back(Triangle(colors[i])); // con color
    }
};

void Mesh::SetScale(float *scale)
{
    this->scale = {scale[0], scale[1], scale[2]};
}

void Mesh::SetRotation(float *rotation)
{
    this->rotation = {rotation[0], rotation[1], rotation[2]};
}

void Mesh::Update()
{
    // Set new scalation, rotation and translation amounts
    rotation += rotationAmount;

    // Loop all triangle faces of the mesh
    for (size_t i = 0; i < triangles.size(); i++)
    {
        // Create a new triangle to store data and render it later
        triangles[i].vertices[0] = vertices[static_cast<int>(faces[i].x) - 1];
        triangles[i].vertices[1] = vertices[static_cast<int>(faces[i].y) - 1];
        triangles[i].vertices[2] = vertices[static_cast<int>(faces[i].z) - 1];

        // preparing matrix transformation
        triangles[i].PrepareTransform();

        /*** Apply transformations for all face vertices ***/
        for (size_t j = 0; j < 3; j++)
        {
            // Scale using the matrix
            triangles[i].ScaleVertex(j, scale);
            // Rotation
            triangles[i].RotateVertex(j, rotation);
            // Translation (away from camera)
            triangles[i].TranslateVertex(j, window->modelPosition);
        }

        /*** Back Face Culling Algorithm ***/
        if (window->enableBackfaceCulling)
        {
            triangles[i].ApplyCulling(window->cameraPosition);
            // Bypass the projection if triangle is being culled

            if (triangles[i].culling)
                continue;
        }

        /*Before project calculate depth*/
        triangles[i].CalculateAverageDepth();

        /*** Apply projections for all face vertices ***/
        for (size_t j = 0; j < 3; j++)
        {
            triangles[i].ProjectVertex(j, window->fovFactor);
            // Translate the projected vertex to the middle screen
            triangles[i].projectedVertices[j].x += (window->windowWidth / 2);
            triangles[i].projectedVertices[j].y += (window->windowHeight / 2);
        }
    }
}

void Mesh::Render()
{
    // Antes de renderizar triángulos ordenarlos por media de profundidad
    std::deque sortedTriangles(triangles);
    std::sort(sortedTriangles.begin(), sortedTriangles.end());

    // Loop projected triangles array and render them
    for (size_t i = 0; i < sortedTriangles.size(); i++)
    {
        // If culling is true and enabled globally bypass the current triangle
        if (window->enableBackfaceCulling && sortedTriangles[i].culling)
            continue;

        // Triángulos
        if (window->drawFilledTriangles)
        {
            window->DrawFilledTriangle(
                sortedTriangles[i].projectedVertices[0].x, sortedTriangles[i].projectedVertices[0].y,
                sortedTriangles[i].projectedVertices[1].x, sortedTriangles[i].projectedVertices[1].y,
                sortedTriangles[i].projectedVertices[2].x, sortedTriangles[i].projectedVertices[2].y,
                sortedTriangles[i].color);
        }

        // Wireframe
        if (window->drawWireframe)
        {
            window->DrawTriangle(
                sortedTriangles[i].projectedVertices[0].x, sortedTriangles[i].projectedVertices[0].y,
                sortedTriangles[i].projectedVertices[1].x, sortedTriangles[i].projectedVertices[1].y,
                sortedTriangles[i].projectedVertices[2].x, sortedTriangles[i].projectedVertices[2].y,
                0xFF0095FF);
        }

        // Vértices
        if (window->drawWireframeDots)
        {
            window->DrawRect(sortedTriangles[i].projectedVertices[0].x - 2, sortedTriangles[i].projectedVertices[0].y - 2, 5, 5, 0xFFFF0000);
            window->DrawRect(sortedTriangles[i].projectedVertices[1].x - 2, sortedTriangles[i].projectedVertices[1].y - 2, 5, 5, 0xFFFF0000);
            window->DrawRect(sortedTriangles[i].projectedVertices[2].x - 2, sortedTriangles[i].projectedVertices[2].y - 2, 5, 5, 0xFFFF0000);
        }
    }
}
