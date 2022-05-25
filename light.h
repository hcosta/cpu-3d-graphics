#ifndef LIGHT_H
#define LIGHT_H

#include "vector.h"

class Light
{
public:
    Vector3 direction;

    static uint32_t ApplyIntensity(uint32_t color, float percentageFactor)
    {
        // Clamp percentageFactor between 0 and 1
        if (percentageFactor < 0)
            percentageFactor = 0;
        if (percentageFactor > 1)
            percentageFactor = 1;

        uint32_t a = (color & 0xFF000000);
        uint32_t r = (color & 0x00FF0000) * percentageFactor;
        uint32_t g = (color & 0x0000FF00) * percentageFactor;
        uint32_t b = (color & 0x000000FF) * percentageFactor;

        uint32_t newColor = a | (r & 0x00FF0000) | (g & 0x0000FF00) | (b & 0x000000FF);
        return newColor;
    }
};

#endif