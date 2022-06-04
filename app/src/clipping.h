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
    std::deque<Vector3> vertices;
    std::deque<Texture2> textureUVCoords;

    Polygon(Triangle triangle)
    {
        // Save the starting triangle vertices
        vertices.push_back(triangle.vertices[0]);
        vertices.push_back(triangle.vertices[1]);
        vertices.push_back(triangle.vertices[2]);

        // Save the starting triangle UV Coords
        textureUVCoords.push_back(triangle.textureUVCoords[0]);
        textureUVCoords.push_back(triangle.textureUVCoords[1]);
        textureUVCoords.push_back(triangle.textureUVCoords[2]);
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

                Triangle clippedTriangle = Triangle(0xFFFFFFFF);

                // Set the vertices
                clippedTriangle.vertices[0] = vertices[index0];
                clippedTriangle.vertices[1] = vertices[index1];
                clippedTriangle.vertices[2] = vertices[index2];

                // Set the texture UV coords
                clippedTriangle.textureUVCoords[0] = textureUVCoords[index0];
                clippedTriangle.textureUVCoords[1] = textureUVCoords[index1];
                clippedTriangle.textureUVCoords[2] = textureUVCoords[index2];

                clippedTriangles.push_back(clippedTriangle);
            }
        }
    }

private:
    static float FloatLerp(float a, float b, float f)
    {
        return a + f * (b - a);
    }

    void ClipAgainstPlane(Plane plane)
    {
        // Creamos una cola para almacenar los vértices dentro del plano
        std::deque<Vector3> insideVertices;
        // Creamos una cola para almacenar las coordenadas UV de las tetxturas dentro del plano
        std::deque<Texture2> insideTextureUVCoords;

        // Recorremos todos los vértices
        for (size_t i = 0; i < vertices.size(); i++)
        {
            // Recuperamos el vértice actual y el anterior
            Vector3 currentVertex = vertices[i];
            // Si recién empezamos (i==0) el anterior será el último
            Vector3 previousVertex = (i > 0) ? vertices[i - 1] : vertices[vertices.size() - 1];

            // Recuperamos las coordenadas UV actuales y anteriores
            Texture2 curTexUVCoords = textureUVCoords[i];
            // Si recién empezamos (i==0) el anterior será el último
            Texture2 prevTexUVCoords = (i > 0) ? textureUVCoords[i - 1] : textureUVCoords[textureUVCoords.size() - 1];

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

                // Calculamos el punto de intersección interpolado, I = Q1 + t(Q2-Q1)
                Vector3 intersectionPoint;
                intersectionPoint.x = FloatLerp(previousVertex.x, currentVertex.x, tFactor);
                intersectionPoint.y = FloatLerp(previousVertex.y, currentVertex.y, tFactor);
                intersectionPoint.z = FloatLerp(previousVertex.z, currentVertex.z, tFactor);

                // Insertamos el nuevo punto de intersección a la lista de vértices internos
                insideVertices.push_back(intersectionPoint);

                // Calculamos las coordenadas de las texturas UV interpoladas
                Texture2 interpolatedTexUVCoord;
                interpolatedTexUVCoord.u = FloatLerp(prevTexUVCoords.u, curTexUVCoords.u, tFactor);
                interpolatedTexUVCoord.v = FloatLerp(prevTexUVCoords.v, curTexUVCoords.v, tFactor);

                // Insertamos las nueva coordenadas de la textura interpolada
                insideTextureUVCoords.push_back(interpolatedTexUVCoord);
            }

            // Si el vértice se encuentra dentro del plano
            if (currentDot > 0)
            {
                // Lo añadimos a la cola
                insideVertices.push_back(currentVertex);
                // Y también añadimos la textura
                insideTextureUVCoords.push_back(curTexUVCoords);
            }
        }

        // Copiamos los vértices dentro del plano a los actuales
        vertices.clear();
        vertices = insideVertices;

        // Copiamos las coordenadas de las texturas UV dentro del plano a las actuales
        textureUVCoords.clear();
        textureUVCoords = insideTextureUVCoords;
    }
};

#endif