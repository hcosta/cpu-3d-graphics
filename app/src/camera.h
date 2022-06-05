#ifndef CAMERA_H
#define CAMERA_H

#include "vector.h"
#include "camera.h"

class Camera {
public:
    Vector3 position{ 0, 0, 0 };
    Vector3 direction{ 0, 0, 0 };
    Vector3 forwardVelocity{ 0, 0, 0 };
    Vector3 sideVelocity{ 0, 0, 0 };
    Vector3 verticalVelocity{ 0, 0, 0 };
    float yawPitch[2]{ 0,0 };

    Vector3 GetTarget()
    {
        //// LOOKAT CAMERA VIEW MATRIX WITH HARDCODED TARGET
        // Vector3 target = { window->modelTranslation[0], window->modelTranslation[1], window->modelTranslation[2] };
        // window->viewMatrix = Matrix4::LookAt(window->camera.position, target, upDirection);

        //// FPS CAMERA VIEW MATRIX WITHOUT HARDCODED TARGET
        
        // Create an initial target vector forward the z-axis
        Vector3 target = { 0, 0, 1 };
        // Calculate yaw rotation matrix and set the direction
        Matrix4 cameraYawRotationMatrix = Matrix4::RotationYMatrix(yawPitch[0]);
        Matrix4 cameraPitchRotationMatrix = Matrix4::RotationXMatrix(yawPitch[1]);
        direction = target * cameraPitchRotationMatrix * cameraYawRotationMatrix;
        // Offset the camera position in the direction where the camera is pointint at
        target = position + direction;
        
        return target;
    }
};

#endif