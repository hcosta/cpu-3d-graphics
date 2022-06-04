#include "mesh.h"
#include "window.h" // Importamos la fuente de la ventana
#include "clipping.h"
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
    this->translation = { translation[0], translation[1], translation[2] };
}

void Mesh::Update()
{
    // Clear all the clippedTriangles for the current frame
    clippedTriangles.clear();

    //// LOOKAT CAMERA VIEW MATRIX WITH HARDCODED TARGET
    // Vector3 target = { window->modelTranslation[0], window->modelTranslation[1], window->modelTranslation[2] };
    // window->viewMatrix = Matrix4::LookAt(window->camera.position, target, upDirection);

    //// FPS CAMERA VIEW MATRIX WITHOUT HARDCODED TARGET
    // Create an initial target vector forward the z-axis
    Vector3 target = {0, 0, 1};  
    // Calculate yaw rotation matrix and set the direction
    Matrix4 cameraYawRotationMatrix = Matrix4::RotationYMatrix(window->camera.yawPitch[0]);
    Matrix4 cameraPitchRotationMatrix = Matrix4::RotationXMatrix(window->camera.yawPitch[1]);
    window->camera.direction = target * cameraPitchRotationMatrix * cameraYawRotationMatrix;
    // Offset the camera position in the direction where the camera is pointint at
    target = window->camera.position + window->camera.direction;
    Vector3 upDirection = { 0, 1, 0 };
    // Calculate the view matrix for each frame
    window->viewMatrix = Matrix4::LookAt(window->camera.position, target, upDirection);

    // Loop all triangle faces of the mesh
    for (size_t i = 0; i < triangles.size(); i++)
    {

        // Create a new triangle to store data and render it later
        triangles[i].vertices[0] = vertices[static_cast<int>(faces[i].x) - 1];
        triangles[i].vertices[1] = vertices[static_cast<int>(faces[i].y) - 1];
        triangles[i].vertices[2] = vertices[static_cast<int>(faces[i].z) - 1];

        /*** Apply world transformation and view transformation for all face vertices ***/
        for (size_t j = 0; j < 3; j++)
        {
            // World transformation to get the world space
            triangles[i].WorldVertexTransform(j, scale, rotation, translation);
            // View transformation to get the view space (aka camera space) 
            triangles[i].ViewVertexTransform(j, window->viewMatrix);
        }

        /*** Back Face Culling Algorithm ***/
        triangles[i].CalculateNormal();
        if (window->enableBackfaceCulling)
        {
            triangles[i].ApplyCulling(&window->camera);
            // Bypass the projection if triangle is being culled

            if (triangles[i].culling)
                continue;
        }

        /*** CLIPPING: BEFORE THE PROJECTION */

        // Create the initial polygon with the triangle face vertices
        Polygon polygon(triangles[i]);
        // Then do the clipping
        polygon.Clip(window->viewFrustum);
        // Add the new triangles to the clippedTriangles dequeue
        polygon.GenerateClippedTriangles(clippedTriangles);
    }

    // PROJECTING
    for (size_t i = 0; i < clippedTriangles.size(); i++)
    {
        /*** Calculate the new triangle normal ***/
        clippedTriangles[i].CalculateNormal();

        /*** Apply projections and lighting for all face vertices ***/
        for (size_t j = 0; j < 3; j++)
        {
            // Project the current vertex using matrices
            clippedTriangles[i].ProjectWorldVertex(j, window->projectionMatrix);
            // First scale the projected vertex by screen sizes
            clippedTriangles[i].projectedVertices[j].x *= (window->rendererWidth / 2.0);
            clippedTriangles[i].projectedVertices[j].y *= (window->rendererHeight / 2.0);
            // Invert the y values to account the flipped screen y coord
            clippedTriangles[i].projectedVertices[j].y *= -1;
            // Then translate the projected vertex to the middle screen
            clippedTriangles[i].projectedVertices[j].x += (window->rendererWidth / 2.0);
            clippedTriangles[i].projectedVertices[j].y += (window->rendererHeight / 2.0);
        }

        // Project the normal vectors if we want to draw it
        if (window->drawTriangleNormals)
        {
            // Project the current normal to create an origin and a destiny vectors
            clippedTriangles[i].ProjectWorldNormal(window->projectionMatrix);
            for (size_t j = 0; j < 2; j++)
            {
                // First scale the projected vertex by screen sizes
                clippedTriangles[i].projectedNormal[j].x *= (window->rendererWidth / 2.0);
                clippedTriangles[i].projectedNormal[j].y *= (window->rendererHeight / 2.0);
                // Invert the y values to account the flipped screen y coord
                clippedTriangles[i].projectedNormal[j].y *= -1;
                // Then translate the projected vertex to the middle screen
                clippedTriangles[i].projectedNormal[j].x += (window->rendererWidth / 2.0);
                clippedTriangles[i].projectedNormal[j].y += (window->rendererHeight / 2.0);
            }
        }

        /** Apply flat shading ***/
        clippedTriangles[i].ApplyFlatShading(window->light);
    }
}

void Mesh::Render()
{
    // Antes de renderizar triángulos ordenarlos por media de profundidad
    // Esto ya no es necesario al estar utilizando un zbuffer
    // std::deque<Triangle> sortedTriangles(triangles);
    // std::sort(sortedTriangles.begin(), sortedTriangles.end());

    // RENDERING: Loop all projected clippedTriangles and render them
    for (size_t i = 0; i < clippedTriangles.size(); i++)
    {
        // If culling is true and enabled globally bypass the current triangle
        if (window->enableBackfaceCulling && clippedTriangles[i].culling)
            continue;

        // Triángulos
        if (window->drawFilledTriangles && !window->drawTexturedTriangles)
        {
            window->DrawFilledTriangle(
                clippedTriangles[i].projectedVertices[0].x, clippedTriangles[i].projectedVertices[0].y, clippedTriangles[i].projectedVertices[0].z, clippedTriangles[i].projectedVertices[0].w,
                clippedTriangles[i].projectedVertices[1].x, clippedTriangles[i].projectedVertices[1].y, clippedTriangles[i].projectedVertices[1].z, clippedTriangles[i].projectedVertices[1].w,
                clippedTriangles[i].projectedVertices[2].x, clippedTriangles[i].projectedVertices[2].y, clippedTriangles[i].projectedVertices[2].z, clippedTriangles[i].projectedVertices[2].w,
                clippedTriangles[i].color);
        }

        // Triángulos texturizados
        if (window->drawTexturedTriangles)
        {
            window->DrawTexturedTriangle(
                clippedTriangles[i].projectedVertices[0].x, clippedTriangles[i].projectedVertices[0].y, clippedTriangles[i].projectedVertices[0].z, clippedTriangles[i].projectedVertices[0].w, clippedTriangles[i].textureUVCoords[0],
                clippedTriangles[i].projectedVertices[1].x, clippedTriangles[i].projectedVertices[1].y, clippedTriangles[i].projectedVertices[1].z, clippedTriangles[i].projectedVertices[1].w, clippedTriangles[i].textureUVCoords[1],
                clippedTriangles[i].projectedVertices[2].x, clippedTriangles[i].projectedVertices[2].y, clippedTriangles[i].projectedVertices[2].z, clippedTriangles[i].projectedVertices[2].w, clippedTriangles[i].textureUVCoords[2],
                meshTexture, textureWidth, textureHeight);
        }

        // Wireframe
        if (window->drawWireframe)
        {
            window->DrawTriangle3D(
                clippedTriangles[i].projectedVertices[0].x, clippedTriangles[i].projectedVertices[0].y, clippedTriangles[i].projectedVertices[0].w,
                clippedTriangles[i].projectedVertices[1].x, clippedTriangles[i].projectedVertices[1].y, clippedTriangles[i].projectedVertices[1].w,
                clippedTriangles[i].projectedVertices[2].x, clippedTriangles[i].projectedVertices[2].y, clippedTriangles[i].projectedVertices[2].w,
                0xFF000000);
        }

        // Triangle normals
        if (window->drawTriangleNormals)
        {
            window->DrawLine3D(
                clippedTriangles[i].projectedNormal[0].x, clippedTriangles[i].projectedNormal[0].y, clippedTriangles[i].projectedNormal[0].w,
                clippedTriangles[i].projectedNormal[1].x, clippedTriangles[i].projectedNormal[1].y, clippedTriangles[i].projectedNormal[1].w,
                0xFF07EB07);
        }

        // Vértices
        if (window->drawWireframeDots)
        {
            window->DrawRect(clippedTriangles[i].projectedVertices[0].x - 1, clippedTriangles[i].projectedVertices[0].y - 1, 3, 3, 0xFF00FFFF);
            window->DrawRect(clippedTriangles[i].projectedVertices[1].x - 1, clippedTriangles[i].projectedVertices[1].y - 1, 3, 3, 0xFF00FFFF);
            window->DrawRect(clippedTriangles[i].projectedVertices[2].x - 1, clippedTriangles[i].projectedVertices[2].y - 1, 3, 3, 0xFF00FFFF);
        }
    }
}
