#pragma once

#include "Hittable.h"

class Sphere : public Hittable {
public:

	Sphere() {
		center = Vec3(0,0,0);
		radius = 0;
	}

	Sphere(Point3 c, double r) {
		center = c;
		radius = r;
	};

	bool hit(const Ray& r, double tMin, double tMax, HitRecord& rec) const override;

public:
	Point3 center;
	double radius;
};

bool Sphere::hit(const Ray& r, double t_min, double t_max, HitRecord& rec) const {
    Vec3 oc = r.getOrigin() - center;
    auto a = r.getDirection().lengthSquared();
    auto halfB = dot(oc, r.getDirection());
    auto c = oc.lengthSquared() - radius * radius;

    auto discriminant = halfB * halfB - a * c;
    if (discriminant < 0) return false;
    auto sqrtd = sqrt(discriminant);

    // Find the nearest root that lies in the acceptable range.
    auto root = (-halfB - sqrtd) / a;
    if (root < t_min || t_max < root) {
        root = (-halfB + sqrtd) / a;
        if (root < t_min || t_max < root)
            return false;
    }

    rec.t = root;
    rec.p = r.at(rec.t);
    rec.normal = (rec.p - center) / radius;
    Vec3 outwardNormal = (rec.p - center) / radius;
    rec.setFaceNormal(r, outwardNormal);

    return true;
}
