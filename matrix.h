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
};

#endif