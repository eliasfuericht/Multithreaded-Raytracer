#pragma once

#include "Utility.h"

class Camera {
public:
	Camera() {
        auto aspect = 16.0 / 9.0;
        auto viewportHeight = 2.0;
        auto viewportWidth = viewportHeight * aspect;
        auto focalLength = 1.0;

        origin = Point3(0, 0, 0);
        horizontal = Vec3(viewportWidth, 0, 0);
        vertical = Vec3(0, viewportHeight, 0);
        lowerLeftCorner = origin - horizontal / 2 - vertical / 2 - Vec3(0, 0, focalLength);
	}

    Camera(Point3 o,const float aspect, const float viewportHeight, const float focalLength) {
        auto viewportWidth = viewportHeight * aspect;

        origin = o;
        horizontal = Vec3(viewportWidth, 0, 0);
        vertical = Vec3(0, viewportHeight, 0);
        lowerLeftCorner = origin - horizontal / 2 - vertical / 2 - Vec3(0, 0, focalLength);
    }

    Ray getRay(double u, double v) const {
        return Ray(origin, lowerLeftCorner + u * horizontal + v * vertical - origin);
    }

public:
    Point3 origin;
    Point3 lowerLeftCorner;
    Vec3 horizontal;
    Vec3 vertical;
};

