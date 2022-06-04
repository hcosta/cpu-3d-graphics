#ifndef CLIPPING_H
#define CLIPPING_H

#include <deque>
#include <math.h>
#include "vector.h"
#include "triangle.h"

class Plane
{
public:
    Vector3 point;
    Vector3 normal;
};

class Frustum
{
public:
    Plane leftPlane;
    Plane rightPlane;
    Plane topPlane;
    Plane bottomPlane;
    Plane nearPlane;
    Plane farPlane;

    Frustum() = default;

    Frustum(float fovFactorX, float fovFactorY, float zNear, float zFar)
    {
        float cosHalfFovX = cos(fovFactorX / 2);
        float sinHalfFovX = sin(fovFactorX / 2);

        float cosHalfFovY = cos(fovFactorY / 2);
        float sinHalfFovY = sin(fovFactorY / 2);

        leftPlane.point = Vector3{ 0, 0, 0 };
        leftPlane.normal = Vector3{ cosHalfFovX, 0, sinHalfFovX };

        rightPlane.point = Vector3{ 0, 0, 0 };
        rightPlane.normal = Vector3{ -cosHalfFovX, 0, sinHalfFovX };

        topPlane.point = Vector3{ 0, 0, 0 }; 
        topPlane.normal = Vector3{ 0, -cosHalfFovY, sinHalfFovY };

        bottomPlane.point = Vector3{ 0, 0, 0 };
        bottomPlane.normal = Vector3{ 0, cosHalfFovY, sinHalfFovY };

        nearPlane.point = Vector3{ 0, 0, zNear };
        nearPlane.normal = Vector3{ 0, 0, 1 };

        farPlane.point = Vector3{ 0, 0, zFar };
        farPlane.normal = Vector3{ 0, 0, -1 };
    }
};

class Polygon
{
public:
    Triangle sourceTriangle;
    std::deque<Vector3> vertices;

    Polygon(Triangle triangle) : sourceTriangle(triangle) 
    {
        // Save the starting triangle vertices
        vertices.push_back(triangle.vertices[0]);
        vertices.push_back(triangle.vertices[1]);
        vertices.push_back(triangle.vertices[2]);
    }

    void Clip(Frustum viewFrustum)
    {
        ClipAgainstPlane(viewFrustum.leftPlane);
        ClipAgainstPlane(viewFrustum.rightPlane);
        ClipAgainstPlane(viewFrustum.topPlane);
        ClipAgainstPlane(viewFrustum.bottomPlane);
        ClipAgainstPlane(viewFrustum.nearPlane);
        ClipAgainstPlane(viewFrustum.farPlane);
    }

    void GenerateClippedTriangles(std::deque<Triangle>& clippedTriangles)
    {
        // Ensure a minimum of 3 vertices to create a new triangle
        if (vertices.size() >= 3)
        {
            for (size_t i = 0; i < vertices.size() - 2; i++)
            {
                int index0 = 0;
                int index1 = i + 1;
                int index2 = i + 2;

                Triangle triangle = Triangle(0xFFFFFFFF);
                triangle.vertices[0] = vertices[index0];
                triangle.vertices[1] = vertices[index1];
                triangle.vertices[2] = vertices[index2];

                clippedTriangles.push_back(triangle);
            }
        }
    }

private:
    void ClipAgainstPlane(Plane plane)
    {
        // Creamos una cola para almacenar los vértices dentro del plano
        std::deque<Vector3> insideVertices;

        // Recorremos todos los vértices
        for (size_t i = 0; i < vertices.size(); i++)
        {
            // Recuperamos el vértice actual y el anterior
            Vector3 currentVertex = vertices[i];
            // Si recién empezamos (i==0) el anterior será el último
            Vector3 previousVertex = (i > 0) ? vertices[i - 1] : vertices[vertices.size() - 1];

            // Calculamos los productos escalares de ambos (dotQ1 = n·(Q1-P))
            float currentDot = (currentVertex - plane.point).DotProduct(plane.normal);
            float previousDot = (previousVertex - plane.point).DotProduct(plane.normal);

            // Si el vértice está fuera del plano calculamos el punto de intersección
            // Podemos saberlo si uno es positivo y el otro es negativo, signigicando esto
            // que un punto a pasado a estar de dentro a fuera o viceversa, de fuera a dentro
            if (currentDot * previousDot < 0)
            {
                // Calculamos el factor de interpolación, t = dotQ1/(dotQ1-dotQ2)
                float tFactor = previousDot / (previousDot - currentDot);
                // Calculamos el punto de intersección, I = Q1 + t(Q2-Q1)
                Vector3 intersectionPoint = currentVertex; // I = Qc
                intersectionPoint -= previousVertex;       // I = (Qc-Qp)
                intersectionPoint *= tFactor;              // I = t(Qc-Qp)
                intersectionPoint += previousVertex;       // I = Qp+t(Qc-Qp)
                // Insertamos el nuevo punto de intersección a la lista de vértices internos
                insideVertices.push_back(intersectionPoint);
            }

            // Si el vértice se encuentra dentro del plano lo añadimos a la cola
            if (currentDot > 0) insideVertices.push_back(currentVertex);
        }

        // Copiamos los vértices dentro del plano a los vértices actuales
        vertices.clear();
        vertices = insideVertices;
    }
};

#endif