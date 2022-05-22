#ifndef MATRIX_H
#define MATRIX_H

#include "vector.h"

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

    static Matrix4 ScaleMatrix(float x, float y, float z)
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
};

#endif