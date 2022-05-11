#include "vector.h"

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