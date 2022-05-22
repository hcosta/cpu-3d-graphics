#ifndef TRIANGLE_H
#define TRIANGLE_H

#include "vector.h"

class Triangle
{
public:
    Vector3 vertices[3];            // 3d  vertices
    Vector2 projectedVertices[3];   // 2d vertices
    Vector4 transformedVertices[3]; // 3d matrix transformations
    uint32_t color = 0xFFFFFFFF;
    bool culling = false;
    float averageDepth;

    Triangle() = default;
    Triangle(uint32_t color) : color(color){};

    bool operator<(const Triangle &t) const
    {
        return averageDepth < t.averageDepth;
    }

    void PrepareTransform()
    {
        transformedVertices[0] = Vector4(vertices[0]);
        transformedVertices[1] = Vector4(vertices[1]);
        transformedVertices[2] = Vector4(vertices[2]);
    }

    void ScaleVertex(int vertexIndex, Vector3 scale)
    {
        // Use a matrix to transform scale the origin vertex
        transformedVertices[vertexIndex] *= Matrix4::ScaleMatrix(scale.x, scale.y, scale.y);
        vertices[vertexIndex] = transformedVertices[vertexIndex].ToVector3();
    }

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

    void ProjectVertex(int vertexIndex, float fovFactor)
    {
        projectedVertices[vertexIndex] = vertices[vertexIndex].PerspectiveProjection(fovFactor);
    };

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

    void CalculateAverageDepth()
    {
        averageDepth = (vertices[0].z + vertices[1].z + vertices[2].z) / 3;
    }
};

#endif