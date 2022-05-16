#ifndef VECTOR_H
#define VECTOR_H

#include <iostream>

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

    Vector2(double x, double y) : x(x), y(y){};
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
    Vector3 operator-(const Vector3 &v) const;
    Vector3 operator*(float factor) const;
    Vector3 operator/(float factor) const;
    Vector3 CrossProduct(const Vector3 &v) const;
    float DotProduct(const Vector3 &v) const;

    Vector3(double x, double y, double z) : x(x), y(y), z(z){};

    float Length();

    void RotateX(float angle);
    void RotateY(float angle);
    void RotateZ(float angle);

    Vector2 OrtoraphicProjection(float fovFactor);
    Vector2 PerspectiveProjection(float fovFactor);

    void Rotate(Vector3 angles);
};

#endif