#pragma once

#include "Vec3.h"

#include <iostream>

void writeColor(std::ostream& out, Color pixelColor, int samplesPerPixel) {
    auto r = pixelColor.getX();
    auto g = pixelColor.getY();
    auto b = pixelColor.getZ();

    // Divide the color by the number of samples.
    auto scale = 1.0 / samplesPerPixel;
    r *= scale;
    g *= scale;
    b *= scale;

    // Write the translated [0,255] value of each color component.
    out << static_cast<int>(256 * clamp(r, 0.0, 0.999)) << ' '
        << static_cast<int>(256 * clamp(g, 0.0, 0.999)) << ' '
        << static_cast<int>(256 * clamp(b, 0.0, 0.999)) << '\n';
}


