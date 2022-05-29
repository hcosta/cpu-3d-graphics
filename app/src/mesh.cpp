#include "mesh.h"
#include "window.h" // Importamos la fuente de la ventana
#include <fstream>
#include <algorithm>
#include <string>
#include <deque>

Mesh::Mesh(Window* window, std::string modelFileName, std::string textureFileName)
{
    this->window = window;

    // Open the file
    std::ifstream modelFile(modelFileName);
    if (!modelFile.is_open())
    {
        std::cerr << "Error reading the file " << modelFileName << std::endl;
        return;
    }

    // If file is loaded in memory read each line
    std::string line;
    while (std::getline(modelFile, line))
    {
        // if starts with v it's a vertex
        if (line.rfind("v ", 0) == 0)
        {
            Vector3 vertex; // %lf -> double (large float)
            sscanf_s(line.c_str(), "v %lf %lf %lf", &vertex.x, &vertex.y, &vertex.z);
            this->vertices.push_back(vertex);
        }
        // if starts with vt it's a texture coordinate
        else if (line.rfind("vt ", 0) == 0)
        {
            Texture2 textureCoords;
            sscanf_s(line.c_str(), "vt %f %f", &textureCoords.u, &textureCoords.v);
            this->coordinates.push_back(textureCoords);
        }
        // if starts with f it's a face
        else if (line.rfind("f ", 0) == 0)
        {
            int vertexIndices[3];
            int textureIndices[3];
            int normalIndices[3];
            sscanf_s(line.c_str(), "f %d/%d/%d %d/%d/%d %d/%d/%d",
                   &vertexIndices[0], &textureIndices[0], &normalIndices[0],
                   &vertexIndices[1], &textureIndices[1], &normalIndices[1],
                   &vertexIndices[2], &textureIndices[2], &normalIndices[2]);
            Vector3 face;
            face.x = vertexIndices[0];
            face.y = vertexIndices[1];
            face.z = vertexIndices[2];
            this->faces.push_back(face);
            // recover the triangle coords using the textureIndeces
            Texture2 triangleCoords[]{ 
                this->coordinates[textureIndices[0] - 1], 
                this->coordinates[textureIndices[1] - 1], 
                this->coordinates[textureIndices[2] - 1]};
            this->triangles.push_back(Triangle(0xFFFFFFFF, triangleCoords));
        }
    }

    // Load the texture after loading the model
    pngTexture = upng_new_from_file(textureFileName.c_str());
    if (pngTexture == nullptr)
    {
        std::cerr << "Error reading the file " << textureFileName << std::endl;
        return;
    }

    upng_decode(pngTexture);
    if (upng_get_error(pngTexture) == UPNG_EOK)
    {
        meshTexture = (uint32_t*)upng_get_buffer(pngTexture);
        textureWidth = upng_get_width(pngTexture);
        textureHeight = upng_get_height(pngTexture);
    }
}

Mesh::Mesh(Window *window, Vector3 *vertices, int verticesLength, Vector3 *faces, int facesLength, uint32_t *colors, Texture2 * textureUVs)
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
        Texture2 triangleTextureUVs[]{ textureUVs[i * 3], textureUVs[i * 3 + 1], textureUVs[i * 3 + 2] };
        this->triangles.push_back(Triangle(colors[i], triangleTextureUVs)); // con color y texturas
    }
}

void Mesh::Free()
{
    upng_free(pngTexture);
}

void Mesh::SetScale(float *scale)
{
    this->scale = {scale[0], scale[1], scale[2]};
}

void Mesh::SetRotation(float *rotation)
{
    this->rotation = {rotation[0], rotation[1], rotation[2]};
}

void Mesh::SetTranslation(float *translation)
{
    // Con rectificación de origen
    this->translation = {
        translation[0] - window->cameraPosition[0],
        translation[1] - window->cameraPosition[1],
        translation[2] - window->cameraPosition[2]};
}

void Mesh::Update()
{
    // Loop all triangle faces of the mesh
    for (size_t i = 0; i < triangles.size(); i++)
    {
        // Create a new triangle to store data and render it later
        triangles[i].vertices[0] = vertices[static_cast<int>(faces[i].x) - 1];
        triangles[i].vertices[1] = vertices[static_cast<int>(faces[i].y) - 1];
        triangles[i].vertices[2] = vertices[static_cast<int>(faces[i].z) - 1];

        /*** Apply world transformation for all face vertices ***/
        for (size_t j = 0; j < 3; j++)
        {
            triangles[i].WorldVertex(j, scale, rotation, translation);
        }

        /*** Calculate the notmal ***/
        triangles[i].CalculateNormal();

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

        /*** Apply projections and lighting for all face vertices ***/
        for (size_t j = 0; j < 3; j++)
        {
            // Project the current vertex using matrices
            triangles[i].ProjectWorldVertex(j, window->projectionMatrix);
            // First scale the projected vertex by screen sizes
            triangles[i].projectedVertices[j].x *= (window->windowWidth / 2.0);
            triangles[i].projectedVertices[j].y *= (window->windowHeight / 2.0);
            // Invert the y values to account the flipped screen y coord
            triangles[i].projectedVertices[j].y *= 1;
            // Then translate the projected vertex to the middle screen
            triangles[i].projectedVertices[j].x += (window->windowWidth / 2.0);
            triangles[i].projectedVertices[j].y += (window->windowHeight / 2.0);
        }

        /** Apply flat shading ***/
        triangles[i].ApplyFlatShading(window->light);
    }
}

void Mesh::Render()
{
    // Antes de renderizar triángulos ordenarlos por media de profundidad
    std::deque<Triangle> sortedTriangles(triangles);
    std::sort(sortedTriangles.begin(), sortedTriangles.end());

    // Loop projected triangles array and render them
    for (size_t i = 0; i < sortedTriangles.size(); i++)
    {
        // If culling is true and enabled globally bypass the current triangle
        if (window->enableBackfaceCulling && sortedTriangles[i].culling)
            continue;

        // Triángulos
        if (window->drawFilledTriangles && !window->drawTexturedTriangles)
        {
            window->DrawFilledTriangle(
                sortedTriangles[i].projectedVertices[0].x, sortedTriangles[i].projectedVertices[0].y,
                sortedTriangles[i].projectedVertices[1].x, sortedTriangles[i].projectedVertices[1].y,
                sortedTriangles[i].projectedVertices[2].x, sortedTriangles[i].projectedVertices[2].y,
                sortedTriangles[i].color);
        }

        // Triángulos texturizados
        if (window->drawTexturedTriangles)
        {
            window->DrawTexturedTriangle(
                sortedTriangles[i].projectedVertices[0].x, sortedTriangles[i].projectedVertices[0].y, sortedTriangles[i].projectedVertices[0].z, sortedTriangles[i].projectedVertices[0].w, sortedTriangles[i].textureUVCoords[0],
                sortedTriangles[i].projectedVertices[1].x, sortedTriangles[i].projectedVertices[1].y, sortedTriangles[i].projectedVertices[1].z, sortedTriangles[i].projectedVertices[1].w, sortedTriangles[i].textureUVCoords[1],
                sortedTriangles[i].projectedVertices[2].x, sortedTriangles[i].projectedVertices[2].y, sortedTriangles[i].projectedVertices[2].z, sortedTriangles[i].projectedVertices[2].w, sortedTriangles[i].textureUVCoords[2],
                meshTexture, textureWidth, textureHeight);
        }

        // Wireframe
        if (window->drawWireframe)
        {
            window->DrawTriangle(
                sortedTriangles[i].projectedVertices[0].x, sortedTriangles[i].projectedVertices[0].y,
                sortedTriangles[i].projectedVertices[1].x, sortedTriangles[i].projectedVertices[1].y,
                sortedTriangles[i].projectedVertices[2].x, sortedTriangles[i].projectedVertices[2].y,
                0xFF000000);
        }

        // Vértices
        if (window->drawWireframeDots)
        {
            window->DrawRect(sortedTriangles[i].projectedVertices[0].x - 1, sortedTriangles[i].projectedVertices[0].y - 1, 5, 5, 0xFFFFFF00);
            window->DrawRect(sortedTriangles[i].projectedVertices[1].x - 1, sortedTriangles[i].projectedVertices[1].y - 1, 5, 5, 0xFFFFFF00);
            window->DrawRect(sortedTriangles[i].projectedVertices[2].x - 1, sortedTriangles[i].projectedVertices[2].y - 1, 5, 5, 0xFFFFFF00);
        }
    }
}
