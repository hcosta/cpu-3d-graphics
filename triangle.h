#ifndef TRIANGLE_H
#define TRIANGLE_H

#include "vector.h"

class Triangle
{
public:
    Vector3 vertices[3];
    Vector2 projectedVertices[3];

    void ProjectVertex(int vertexIndex, float fovFactor)
    {
        projectedVertices[vertexIndex] = vertices[vertexIndex].PerspectiveProjection(fovFactor);
    };

    void RotateVertex(int vertexIndex, Vector3 rotation)
    {
        vertices[vertexIndex].Rotate(rotation);
    }

    void TranslateVertex(int vertexIndex, Vector3 distance)
    {
        vertices[vertexIndex].x -= distance.x;
        vertices[vertexIndex].y -= distance.y;
        vertices[vertexIndex].z -= distance.z;
    }
};

#endif