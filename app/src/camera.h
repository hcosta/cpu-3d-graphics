#ifndef CAMERA_H
#define CAMERA_H

#include "vector.h"

class Camera {
public:
    Vector3 position{ 0, 0, 0 };
    Vector3 direction{ 0, 0, 0 };
    Vector3 forwardVelocity{ 0, 0, 0 };
    Vector3 sideVelocity{ 0, 0, 0 };
    Vector3 verticalVelocity{ 0, 0, 0 };
    float yawPitch[2]{ 0,0 };
};

#endif