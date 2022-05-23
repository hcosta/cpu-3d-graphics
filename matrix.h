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
};

#endif