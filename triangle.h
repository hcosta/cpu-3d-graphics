#ifndef TRIANGLE_H
#define TRIANGLE_H

#include "vector.h"

class Triangle
{
public:
    Vector3 vertices[3];
    Vector2 projectedVertices[3];
    bool culling;

    Triangle() = default;

    void ProjectVertex(int vertexIndex, float fovFactor)
    {
        projectedVertices[vertexIndex] = vertices[vertexIndex].PerspectiveProjection(fovFactor);
    };

    void RotateVertex(int vertexIndex, Vector3 rotation)
    {
        vertices[vertexIndex].Rotate(rotation);
    }

    void TranslateVertex(int vertexIndex, float *distance)
    {
        vertices[vertexIndex].x -= distance[0];
        vertices[vertexIndex].y -= distance[1];
        vertices[vertexIndex].z -= distance[2];
    }

    void ApplyCulling(float *cameraPosition)
    {
        // Get the vector substracion B-A and C - A and normalize 'em
        Vector3 vectorAB = this->vertices[1] - this->vertices[0];
        Vector3 vectorAC = this->vertices[2] - this->vertices[0];
        vectorAB.Normalize();
        vectorAC.Normalize();
        // Compute the face normal (corss product) and normalize it
        // Using our left-handed system (z grows inside the monitor)
        // So we apply have to appky the order: AB x AC
        Vector3 normal = vectorAB.CrossProduct(vectorAC);
        normal.Normalize();
        // Find the vector betweenn a triangle point and camera origin
        Vector3 cameraRay = Vector3(cameraPosition[0], cameraPosition[1], cameraPosition[2]) - this->vertices[0];
        // Calculate how aligned the camera ray is with the face normal
        float dotNormalCamera = normal.DotProduct(cameraRay);
        // Test the dotNormalCamera and render the triangle if is >0
        this->culling = (dotNormalCamera < 0);
    }
};

#endif