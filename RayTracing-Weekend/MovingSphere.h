#pragma once

#include "Utility.h"
#include "Hittable.h"

class MovingSphere : public Hittable {
public:
	MovingSphere() {};
    MovingSphere(
        Point3 cen0, Point3 cen1, double _time0, double _time1, double r, shared_ptr<Material> m)
        : center0(cen0), center1(cen1), time0(_time0), time1(_time1), radius(r), matPtr(m)
    {};

    virtual bool hit(const Ray& r, double tMin, double tMax, HitRecord& rec) const override;

    Point3 center(double time) const;

private:
    Point3 center0;
    Point3 center1;
    double time0;
    double time1;
    double radius;
    shared_ptr<Material> matPtr;
};

bool MovingSphere::hit(const Ray& r, double tMin, double tMax, HitRecord& rec) const {
    Vec3 oc = r.getOrigin() - center(r.getTime());
    auto a = r.getDirection().lengthSquared();
    auto halfB = dot(oc, r.getDirection());
    auto c = oc.lengthSquared() - radius * radius;

    auto discriminant = halfB * halfB - a * c;
    if (discriminant < 0) return false;
    auto sqrtd = sqrt(discriminant);

    // Find the nearest root that lies in the acceptable range.
    auto root = (-halfB - sqrtd) / a;
    if (root < tMin || tMax < root) {
        root = (-halfB + sqrtd) / a;
        if (root < tMin || tMax < root)
            return false;
    }

    rec.t = root;
    rec.p = r.at(rec.t);
    rec.normal = (rec.p - center(r.getTime())) / radius;
    Vec3 outwardNormal = (rec.p - center(r.getTime())) / radius;
    rec.setFaceNormal(r, outwardNormal);
    rec.matPtr = matPtr;

    return true;
}

Point3 MovingSphere::center(double time) const {
    return center0 + ((time - time0) / (time1 - time0)) * (center1 - center0);
}