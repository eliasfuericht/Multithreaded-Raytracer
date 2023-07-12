#pragma once

#include "Vec3.h"

#include <iostream>

void write_color(std::ostream& out, Color pixel_color) {
    // Write the translated [0,255] value of each color component.
    out << static_cast<int>(255.999 * pixel_color.getX()) << ' '
        << static_cast<int>(255.999 * pixel_color.getY()) << ' '
        << static_cast<int>(255.999 * pixel_color.getZ()) << '\n';
}


