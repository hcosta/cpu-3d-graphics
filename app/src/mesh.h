#ifndef MESH_H
#define MESH_H

#include <iostream>
#include <vector>
#include <deque>
#include "vector.h"
#include "triangle.h"
#include "upng.h"

// Para prevenir dependencias cíclicas
class Window;

class Mesh
{
public:
    Vector3 scale{1, 1, 1};
    Vector3 rotation{0, 0, 0};
    Vector3 rotationAmount{0, 0, 0};
    Vector3 translation{0, 0, 0};
    std::vector<Vector3> vertices;
    std::vector<Texture2> coordinates;

private:
    Window* window{ nullptr };
    std::vector<Vector3> faces;
    std::vector<Triangle> triangles;
    std::vector<Triangle> clippedTriangles;

    int textureWidth{ 0 };
    int textureHeight{ 0 };
    upng_t* pngTexture{ nullptr };
    uint32_t* meshTexture{ nullptr };

public:
    Mesh() = default;
    Mesh(Window *window, std::string modelFileName, std::string textureFileName, Vector3 scale, Vector3 rotation, Vector3 translation);
    Mesh(Window *window, Vector3 *vertices, int verticesLength, Vector3 *faces, int facesLength, uint32_t *colors, Texture2 *textures);
    void Free();
    void SetScale(float *scale);
    void SetRotation(float *rotation);
    void SetTranslation(float *translation);
    void Update();
    void Render();
};

#endif