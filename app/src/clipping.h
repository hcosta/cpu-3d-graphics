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

    Frustum(float fov, float zNear, float zFar)
    {
        float cosHalfFov = cos(fov / 2);
        float sinHalfFov = sin(fov / 2);

        leftPlane.point = Vector3{ 0,0,0 };
        leftPlane.normal = Vector3{ cosHalfFov,0,sinHalfFov };

        rightPlane.point = Vector3{ 0,0,0 };
        rightPlane.normal = Vector3{ -cosHalfFov,0,sinHalfFov };

        topPlane.point = Vector3{ 0,0,0 };
        topPlane.normal = Vector3{ 0,-cosHalfFov,sinHalfFov };

        bottomPlane.point = Vector3{ 0,0,0 };
        bottomPlane.normal = Vector3{ 0,cosHalfFov,sinHalfFov };

        nearPlane.point = Vector3{ 0,0,zNear };
        bottomPlane.normal = Vector3{ 0,0,1 };

        farPlane.point = Vector3{ 0,0,zFar };
        bottomPlane.normal = Vector3{ 0,0,-1 };
    }
};


class Polygon
{
public:
    std::deque<Vector3> vertices;

    Polygon(Triangle triangle)
    {
        // Save the starting triangle vertices
        this->vertices.push_back(triangle.vertices[0]);
        this->vertices.push_back(triangle.vertices[1]);
        this->vertices.push_back(triangle.vertices[2]);
    }

    void Clip(Frustum viewFrustum)
    {
        ClipAgainstPlane(viewFrustum.leftPlane);
        ClipAgainstPlane(viewFrustum.rightPlane);
        ClipAgainstPlane(viewFrustum.topPlane);
        ClipAgainstPlane(viewFrustum.bottomPlane);
        ClipAgainstPlane(viewFrustum.nearPlane);
        ClipAgainstPlane(viewFrustum.bottomPlane);
    }

private:
    void ClipAgainstPlane(Plane plane)
    {
        // Lógica del recorte
    }
};

#endif