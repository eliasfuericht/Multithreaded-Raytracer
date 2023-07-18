#pragma once

#include "Utility.h"

class Material;

struct HitRecord {
	Point3 p;
	Vec3 normal;
	shared_ptr<Material> matPtr;
	double t;
	bool frontFace;

	inline void setFaceNormal(const Ray& r, const Vec3& outwardNormal) {
		frontFace = dot(r.getDirection(), outwardNormal) < 0;
		normal = frontFace ? outwardNormal : -outwardNormal;
	}
};

class Hittable {
public: 
	virtual bool hit(const Ray& r, double tMin, double tMax, HitRecord& rec) const = 0;
};
