#ifndef VECTOR_H
#define VECTOR_H

#include <iostream>
#include "matrix.h"

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
};

class Vector4
{
public:
    double x{0};
    double y{0};
    double z{0};
    double w{0};

    Vector4() = default;
    Vector4(Vector3 v) : x(v.x), y(v.y), z(v.z), w(1){};
    Vector3 ToVector3();

    Vector4 operator*(Matrix4 m) const;
    Vector4 &operator*=(const Matrix4 &m);
};

#endif