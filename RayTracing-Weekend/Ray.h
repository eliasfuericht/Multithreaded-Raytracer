#pragma once

#include "Vec3.h"

class Ray {
public: 
	Ray() {};
	Ray(const Point3& origin, const Vec3& direction, double time = 0.0)
		: orig(origin), dir(direction), tm(time)
	{}

	Point3 getOrigin() const { return orig; }
	Point3 getDirection() const { return dir; }
	double getTime() const { return tm; }

	Point3 at(double t) const {
		return orig + t * dir;
	}

	Point3 orig;
	Vec3 dir;
	double tm;
};

