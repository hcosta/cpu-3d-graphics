#ifndef VECTOR_H
#define VECTOR_H

#include <iostream>

class Matrix4; /* Pre declaration */

class Vector2
{
public:
    double x;
    double y;

    friend std::ostream &operator<<(std::ostream &os, const Vector2 &v);
    Vector2 operator+(const Vector2 &v) const;
    Vector2 operator-(const Vector2 &v) const;
    Vector2 operator*(float factor) const;
    Vector2 operator/(float factor) const;

    Vector2() = default;
    Vector2(double x, double y) : x(x), y(y){};
    void Normalize();
    float DotProduct(const Vector2 &v) const;
    float Length();
};

class Vector3
{
public:
    double x;
    double y;
    double z;

    friend std::ostream &operator<<(std::ostream &os, const Vector3 &v);
    Vector3 operator+(const Vector3 &v) const;
    Vector3 &operator+=(const Vector3 &v);
    Vector3 operator-(const Vector3 &v) const;
    Vector3 operator*(float factor) const;
    Vector3 operator*(Matrix4 m) const;
    Vector3 operator/(float factor) const;
    Vector3 CrossProduct(const Vector3 &v) const;
    float DotProduct(const Vector3 &v) const;

    Vector3() = default;
    Vector3(double x, double y, double z) : x(x), y(y), z(z){};
    void Normalize();
    void Rotate(Vector3 angles);
    float Length();

    void RotateX(float angle);
    void RotateY(float angle);
    void RotateZ(float angle);

    Vector2 OrtoraphicProjection(float fovFactor);
    Vector2 PerspectiveProjection(float fovFactor);

    static Vector3 BarycentricWeights(Vector2 a, Vector2 b, Vector2 c, Vector2 p)
    {
        // Find vectores between the vertices ABC and point P
        Vector2 ab = b - a;
        Vector2 bc = c - b;
        Vector2 ac = c - a;
        Vector2 ap = p - a;
        Vector2 bp = p - b;

        // Calculate the full triangle ABC area using cross product (area of paralelogram)
        float areaTriangleAbc = (ab.x * ac.y - ab.y * ac.x);

        // Weight alpha is area of subtriangle BCP divided by area of full triangle ABC
        float alpha = (bc.x * bp.y - bp.x * bc.y) / areaTriangleAbc;

        // Weight beta is area of subtriangle ACP divided by area of full triangle ABC
        float beta = (ap.x * ac.y - ac.x * ap.y) / areaTriangleAbc;

        // Wieght gamma is found really easy
        float gamma = 1 - alpha - beta;

        Vector3 weights = { alpha, beta, gamma };
        return weights;
    }
};

class Vector4
{
public:
    double x{0};
    double y{0};
    double z{0};
    double w{0};

    Vector4() = default;
    Vector4(double x, double y, double z, double w) : x(x), y(y), z(z), w(w) {};
    Vector4(Vector3 v) : x(v.x), y(v.y), z(v.z), w(1){};
    Vector3 ToVector3();
    Vector2 ToVector2();

    Vector4 operator*(Matrix4 m) const;
};

#endif