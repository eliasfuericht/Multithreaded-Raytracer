#pragma once

#include "Utility.h"

class AABB {
public:
	AABB() {}
	AABB(const Point3& a, const Point3& b)
		: minimum(a), maximum(b) 
	{}

	Point3 getMinimum() const { return minimum; };
	Point3 getMaximum() const { return maximum; };

    bool hit(const Ray& r, double tMin, double tMax) const;

public:
	Point3 minimum;
	Point3 maximum;
};

inline bool AABB::hit(const Ray& r, double tMin, double tMax) const {
    for (int a = 0; a < 3; a++) {
        auto invD = 1.0f / r.getDirection()[a];
        auto t0 = (getMinimum()[a] - r.getOrigin()[a]) * invD;
        auto t1 = (getMaximum()[a] - r.getOrigin()[a]) * invD;
        if (invD < 0.0f)
            std::swap(t0, t1);
        tMin = t0 > tMin ? t0 : tMin;
        tMax = t1 < tMax ? t1 : tMax;
        if (tMax <= tMin)
            return false;
    }
    return true;
}

AABB surroundingBox(AABB box0, AABB box1) {
    Point3 little(fmin(box0.getMinimum().getX(), box1.getMinimum().getX()),
        fmin(box0.getMinimum().getY(), box1.getMinimum().getY()),
        fmin(box0.getMinimum().getZ(), box1.getMinimum().getZ()));

    Point3 big(fmax(box0.getMaximum().getX(), box1.getMaximum().getX()),
        fmax(box0.getMaximum().getY(), box1.getMaximum().getY()),
        fmax(box0.getMaximum().getZ(), box1.getMaximum().getZ()));

    return AABB(little, big);
}
