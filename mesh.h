#ifndef MESH_H
#define MESH_H

#include <iostream>
#include <deque>
#include "vector.h"
#include "triangle.h"

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
    Window *window;

    std::deque<Vector3> faces;
    std::deque<Vector3> vertices;
    std::deque<Triangle> triangles;

public:
    Mesh() = default;
    Mesh(Window *window, std::string fileName);
    Mesh(Window *window, Vector3 *vertices, int verticesLength, Vector3 *faces, int facesLength, uint32_t *colors);
    void SetScale(float *scale);
    void SetRotation(float *rotation);
    void SetTranslation(float *translation);
    void Update();
    void Render();
};

#endif