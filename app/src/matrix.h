#ifndef MATRIX_H
#define MATRIX_H

#include "vector.h"
#include <math.h>

class Matrix4
{
public:
    float m[4][4];

    static Matrix4 IdentityMatrix()
    {
        //  |  1  0  0  0 |
        //  |  0  1  0  0 |
        //  |  0  0  1  0 |
        //  |  0  0  0  1 |

        return Matrix4{{{1, 0, 0, 0},
                        {0, 1, 0, 0},
                        {0, 0, 1, 0},
                        {0, 0, 0, 1}}};
    }

    static Matrix4 ScalationMatrix(float x, float y, float z)
    {
        //  | sx  0  0  0 |
        //  |  0 sy  0  0 |
        //  |  0  0 sz  0 |
        //  |  0  0  0  1 |
        Matrix4 m = Matrix4::IdentityMatrix();
        m.m[0][0] = x;
        m.m[1][1] = y;
        m.m[2][2] = z;
        return m;
    }

    static Matrix4 TranslationMatrix(float x, float y, float z)
    {
        //  |  1  0  0  tx  |
        //  |  0  1  0  ty  |
        //  |  0  0  1  tz  |
        //  |  0  0  0   1  |
        Matrix4 m = Matrix4::IdentityMatrix();
        m.m[0][3] = x;
        m.m[1][3] = y;
        m.m[2][3] = z;
        return m;
    }

    static Matrix4 RotationXMatrix(float angle)
    {
        float c = cos(angle);
        float s = sin(angle);
        //  |  1  0  0  0  |
        //  |  0  c -s  0  |
        //  |  0  s  c  0  |
        //  |  0  0  0  1  |
        Matrix4 m = Matrix4::IdentityMatrix();
        m.m[1][1] = c;
        m.m[1][2] = -s;
        m.m[2][1] = s;
        m.m[2][2] = c;
        return m;
    }

    static Matrix4 RotationYMatrix(float angle)
    {
        float c = cos(angle);
        float s = sin(angle);
        //  |  c  0  s  0  |
        //  |  0  1  0  0  |
        //  | -s  0  c  0  |
        //  |  0  0  0  1  |
        Matrix4 m = Matrix4::IdentityMatrix();
        m.m[0][0] = c;
        m.m[0][2] = s;
        m.m[2][0] = -s;
        m.m[2][2] = c;
        return m;
    }

    static Matrix4 RotationZMatrix(float angle)
    {
        float c = cos(angle);
        float s = sin(angle);
        //  |  c -s  0  0  |
        //  |  s  c  0  0  |
        //  |  0  0  1  0  |
        //  |  0  0  0  1  |
        Matrix4 m = Matrix4::IdentityMatrix();
        m.m[0][0] = c;
        m.m[0][1] = -s;
        m.m[1][0] = s;
        m.m[1][1] = c;
        return m;
    }

    static Matrix4 WorldMatrix(Vector3 scale, Vector3 angle, Vector3 translate)
    {
        Matrix4 worldMatrix = Matrix4::IdentityMatrix();
        /* El orden de la multiplicación importa ROTACIOn * MUNDO */
        worldMatrix = Matrix4::ScalationMatrix(scale.x, scale.y, scale.z) * worldMatrix;
        worldMatrix = Matrix4::RotationXMatrix(angle.x) * worldMatrix;
        worldMatrix = Matrix4::RotationYMatrix(angle.y) * worldMatrix;
        worldMatrix = Matrix4::RotationZMatrix(angle.z) * worldMatrix;
        worldMatrix = Matrix4::TranslationMatrix(translate.x, translate.y, translate.z) * worldMatrix;
        return worldMatrix;
    }

    static Matrix4 PerspectiveMatrix(float fov, float aspect, float znear, float zfar)
    {
        // | (h/w)*1/tan(fov/2)                0             0                   0 |
        // |                  0     1/tan(fov/2)             0                   0 |
        // |                  0                0    zf/(zf/zn)    (-zf*zn)/(zf-zn) |
        // |                  0                0             1                   0 |
        Matrix4 m = {{{0}}};
        m.m[0][0] = aspect * (1 / tan(fov / 2));
        m.m[1][1] = 1 / tan(fov / 2);
        m.m[2][2] = zfar / (zfar - znear);
        m.m[2][3] = (-zfar * znear) / (zfar - znear);
        m.m[3][2] = 1.0;
        return m;
    }

    static Vector4 ProjectMatrix(Matrix4 perspectiveMatrix, Vector4 originalVector)
    {
        // Multiplicar la matriz de proyección por el vector original
        Vector4 result = originalVector * perspectiveMatrix;

        // Realizar la brecha de perspectiva con el valor original de z guardado en w
        if (result.w != 0.0)
        {
            result.x /= result.w;
            result.y /= result.w;
            result.z /= result.w;
        }
        return result;
    }

    Matrix4 operator*(Matrix4 m2) const
    {
        Matrix4 result;
        for (size_t i = 0; i < 4; i++)
        {
            for (size_t j = 0; j < 4; j++)
            {
                result.m[i][j] = m[i][0] * m2.m[0][j] +
                                 m[i][1] * m2.m[1][j] +
                                 m[i][2] * m2.m[2][j] +
                                 m[i][3] * m2.m[3][j];
            }
        }
        return result;
    }

    static Matrix4 LookAt(Vector3 eye, Vector3 target, Vector3 up)
    {
        // Forward (z) vector in new coordinate system
        Vector3 z = target - eye;
        // Right (x) vector in new coordinate system
        Vector3 x = up.CrossProduct(z);
        // Up (y) vector in new coordinate system
        Vector3 y = z.CrossProduct(x);

        // Normalize the vectors
        x.Normalize();
        y.Normalize();
        z.Normalize();

        // | x.x   x.y   x.z   -dot(x.eye) |
        // | y.x   y.y   y.z   -dot(y.eye) |
        // | z.x   z.y   z.z   -dot(z.eye) |
        // |   0     0     0             1 |

        Matrix4 viewMatrix = {{
            { x.x, x.y, x.z, -x.DotProduct(eye) },
            { y.x, y.y, y.z, -y.DotProduct(eye) },
            { z.x, z.y, z.z, -z.DotProduct(eye) },
            {   0,   0,   0,                  1 },
        }};

        return viewMatrix;
    }
};

#endif