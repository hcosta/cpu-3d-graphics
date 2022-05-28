#ifndef MESH_H
#define MESH_H

#include <iostream>
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

private:
    Window* window{ nullptr };

    std::deque<Vector3> faces;
    std::deque<Vector3> vertices;
    std::deque<Triangle> triangles;
    std::deque<Texture2> coordinates;

    int textureWidth{ 0 };
    int textureHeight{ 0 };
    upng_t* pngTexture{ nullptr };
    uint32_t* meshTexture{ nullptr };

public:
    Mesh() = default;
    Mesh(Window *window, std::string modelFileName, std::string textureFileName);
    Mesh(Window *window, Vector3 *vertices, int verticesLength, Vector3 *faces, int facesLength, uint32_t *colors, Texture2 *textures);
    void Free();
    void SetScale(float *scale);
    void SetRotation(float *rotation);
    void SetTranslation(float *translation);
    void Update();
    void Render();
};

#endif