#pragma once

#include <cmath>
#include <limits>
#include <memory>
#include <cstdlib>

using std::shared_ptr;
using std::make_shared;
using std::sqrt;

const double infinity = std::numeric_limits<double>::infinity();
const double pi = 3.1415926535897932385;

inline double degToRad(double degrees) {
    return degrees * pi / 180.0;
}

inline double randomD() {
    return rand() / (RAND_MAX + 1.0);
}

inline double randomD(double min, double max) {
    return min + (max - min) * randomD();
}

inline double clamp(double x, double min, double max) {
    if (x < min) return min;
    if (x > max) return max;
    return x;
}

#include "vec3.h"
#include "ray.h"