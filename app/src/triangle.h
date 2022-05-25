#ifndef TRIANGLE_H
#define TRIANGLE_H

#include "vector.h"
#include "matrix.h"
#include "light.h"

class Triangle
{
public:
    Vector3 normal{ 0,0,0 };
    Vector3 vertices[3]{}; // 3d  vertices
    // Vector2 projectedVertices[3];   // 2d vertices
    Vector4 projectedVertices[3]{}; // 2d vertices
    uint32_t color{ 0xFFFFFFFF };
    uint32_t originalColor{ color };
    bool culling{ false };
    float averageDepth{ 0 };

    Triangle() = default;
    Triangle(uint32_t color) : color(color), originalColor(color){};

    bool operator<(const Triangle &t) const
    {
        return averageDepth < t.averageDepth;
    }

    void ScaleVertex(int vertexIndex, Vector3 scale)
    {
        // Use a matrix to transform scale the original vertex
        Vector4 transformedVertex{vertices[vertexIndex]};
        transformedVertex = transformedVertex * Matrix4::ScalationMatrix(scale.x, scale.y, scale.z);
        vertices[vertexIndex] = transformedVertex.ToVector3();
    }

    void RotateVertex(int vertexIndex, Vector3 rotation)
    {
        // Use a matrix to transform rotate the original vertex
        Vector4 transformedVertex{vertices[vertexIndex]};
        transformedVertex = transformedVertex * Matrix4::RotationXMatrix(rotation.x);
        transformedVertex = transformedVertex * Matrix4::RotationYMatrix(rotation.y);
        transformedVertex = transformedVertex * Matrix4::RotationZMatrix(rotation.z);
        vertices[vertexIndex] = transformedVertex.ToVector3();
    }

    void TranslateVertex(int vertexIndex, Vector3 translation)
    {
        // Use a matrix to transform translate the original vertex
        Vector4 transformedVertex{vertices[vertexIndex]};
        transformedVertex = transformedVertex * Matrix4::TranslationMatrix(translation.x, translation.y, translation.z);
        vertices[vertexIndex] = transformedVertex.ToVector3();
    }

    void WorldVertex(int vertexIndex, Vector3 scale, Vector3 angle, Vector3 translate)
    {
        // Use a matrix to world transform the original vertex
        Vector4 transformedVertex{vertices[vertexIndex]};
        transformedVertex = transformedVertex * Matrix4::WorldMatrix(scale, angle, translate);
        vertices[vertexIndex] = transformedVertex.ToVector3();
    }

    // void ProjectVertex(int vertexIndex, float fovFactor)
    // {
    //     // Project the original vertex
    //     projectedVertices[vertexIndex] = vertices[vertexIndex].PerspectiveProjection(fovFactor);
    // };

    void ProjectWorldVertex(int vertexIndex, Matrix4 projectionMatrix)
    {
        // Use a matrix to world project the original vertex
        Vector4 transformedVertex{vertices[vertexIndex]};
        projectedVertices[vertexIndex] = Matrix4::ProjectMatrix(projectionMatrix, transformedVertex);
    };

    void ApplyCulling(float *cameraPosition)
    {
        // Find the vector betweenn a triangle point and camera origin
        Vector3 cameraRay = Vector3(cameraPosition[0], cameraPosition[1], cameraPosition[2]) - this->vertices[0];
        // Calculate how aligned the camera ray is with the face normal
        float dotNormalCamera = normal.DotProduct(cameraRay);
        // Test the dotNormalCamera and render the triangle if is >0
        this->culling = (dotNormalCamera < 0);
    }

    void CalculateNormal()
    {
        // Get the vector substracion B-A and C - A and normalize 'em
        Vector3 vectorAB = this->vertices[1] - this->vertices[0];
        Vector3 vectorAC = this->vertices[2] - this->vertices[0];
        vectorAB.Normalize();
        vectorAC.Normalize();
        // Compute the face normal (corss product) and normalize it
        // Using our left-handed system (z grows inside the monitor)
        // So we apply have to appky the order: AB x AC
        normal = vectorAB.CrossProduct(vectorAC);
        normal.Normalize();
    }

    void CalculateAverageDepth()
    {
        averageDepth = (vertices[0].z + vertices[1].z + vertices[2].z) / 3;
    }

    void ApplyFlatShading(Light light)
    {
        // Calculate shading intensity based in how aligned is
        // the normal vector and the inverse vector of the light ray
        float lightIntensityFactor = -normal.DotProduct(light.direction);
        color = Light::ApplyIntensity(originalColor, lightIntensityFactor);
    }
};

#endif