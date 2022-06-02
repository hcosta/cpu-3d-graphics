#include <math.h>
#include "vector.h"
#include "matrix.h"

std::ostream &operator<<(std::ostream &os, const Vector2 &v)
{
    os << "(" << v.x << ", " << v.y << ")";
    return os;
}

std::ostream &operator<<(std::ostream &os, const Vector3 &v)
{
    os << "(" << v.x << ", " << v.y << ", " << v.z << ")";
    return os;
}

float Vector2::Length()
{
    return sqrt(x * x + y * y);
}

void Vector2::Normalize()
{
    float length = Length();
    x = x / length;
    y = y / length;
}

Vector2 Vector2::operator+(const Vector2 &v) const
{
    return Vector2(x + v.x, y + v.y);
}

Vector2 Vector2::operator-(const Vector2 &v) const
{
    return Vector2(x - v.x, y - v.y);
}

Vector2 Vector2::operator*(float factor) const
{
    return Vector2(x * factor, y * factor);
}

Vector2 Vector2::operator/(float factor) const
{
    return Vector2(x / factor, y / factor);
}

float Vector2::DotProduct(const Vector2 &v) const
{
    return (x * v.x) + (y * v.y);
}

float Vector3::Length()
{
    return sqrt(x * x + y * y + z * z);
}

void Vector3::Normalize()
{
    float length = Length();
    x = x / length;
    y = y / length;
    z = z / length;
}

Vector3 Vector3::operator+(const Vector3 &v) const
{
    return Vector3(x + v.x, y + v.y, z + v.z);
}

Vector3 &Vector3::operator+=(const Vector3 &v)
{
    x += v.x;
    y += v.y;
    z += v.z;
    return *this;
}

Vector3& Vector3::operator-=(const Vector3& v)
{
    x -= v.x;
    y -= v.y;
    z -= v.z;
    return *this;
}

Vector3 Vector3::operator-(const Vector3 &v) const
{
    return Vector3(x - v.x, y - v.y, z - v.z);
}

Vector3 Vector3::operator*(float factor) const
{
    return Vector3(x * factor, y * factor, z * factor);
}


Vector3 Vector3::operator*(Matrix4 m) const
{
    Vector3 result;
    result.x = m.m[0][0] * x + m.m[0][1] * y + m.m[0][2] * z ;
    result.y = m.m[1][0] * x + m.m[1][1] * y + m.m[1][2] * z;
    result.z = m.m[2][0] * x + m.m[2][1] * y + m.m[2][2] * z;
    return result;
}

Vector3 Vector3::operator/(float factor) const
{
    return Vector3(x / factor, y / factor, z / factor);
}

Vector3 Vector3::CrossProduct(const Vector3 &v) const
{
    return Vector3(
        y * v.z - z * v.y,
        z * v.x - x * v.z,
        x * v.y - y * v.x);
}

float Vector3::DotProduct(const Vector3 &v) const
{
    return (x * v.x) + (y * v.y) + (z * v.z);
}

void Vector3::RotateX(float angle)
{
    double newY = y * cos(angle) - z * sin(angle);
    double newZ = y * sin(angle) + z * cos(angle);

    y = newY;
    z = newZ;
}

void Vector3::RotateY(float angle)
{
    double newX = x * cos(angle) - z * sin(angle);
    double newZ = x * sin(angle) + z * cos(angle);

    x = newX;
    z = newZ;
}

void Vector3::RotateZ(float angle)
{
    double newX = x * cos(angle) - y * sin(angle);
    double newY = x * sin(angle) + y * cos(angle);

    x = newX;
    y = newY;
}

void Vector3::Rotate(Vector3 angles)
{
    if (angles.x != 0)
        RotateX(angles.x);
    if (angles.y != 0)
        RotateY(angles.y);
    if (angles.z != 0)
        RotateZ(angles.z);
}

Vector2 Vector3::OrtoraphicProjection(float fovFactor)
{
    return Vector2{fovFactor * x, fovFactor * y};
}

Vector2 Vector3::PerspectiveProjection(float fovFactor)
{
    return Vector2{(fovFactor * x) / z, (fovFactor * y) / z};
}

Vector2 Vector4::ToVector2()
{
    return Vector2(x, y);
}

Vector3 Vector4::ToVector3()
{
    return Vector3(x, y, z);
}

Vector4 Vector4::operator*(Matrix4 m) const
{
    Vector4 result;
    result.x = m.m[0][0] * x + m.m[0][1] * y + m.m[0][2] * z + m.m[0][3] * w;
    result.y = m.m[1][0] * x + m.m[1][1] * y + m.m[1][2] * z + m.m[1][3] * w;
    result.z = m.m[2][0] * x + m.m[2][1] * y + m.m[2][2] * z + m.m[2][3] * w;
    result.w = m.m[3][0] * x + m.m[3][1] * y + m.m[3][2] * z + m.m[3][3] * w;
    return result;
}