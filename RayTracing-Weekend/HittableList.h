#pragma once

#include <memory>
#include <vector>

#include "Hittable.h"
#include "AABB.h"

using std::shared_ptr;
using std::make_shared;

class HittableList : public Hittable {
public:
	HittableList() : objects() {}
	HittableList(shared_ptr<Hittable> obj) { add(obj); }

	void clear() { objects.clear(); }
	void add(shared_ptr<Hittable> obj) { objects.push_back(obj); }

	virtual bool hit(const Ray& r, double t_min, double t_max, HitRecord& rec) const override;
    virtual bool boundingBox(double time0, double time1, AABB& output_box) const override;

public:
	std::vector<shared_ptr<Hittable>> objects;
};

bool HittableList::hit(const Ray& r, double t_min, double t_max, HitRecord& rec) const {
    HitRecord temp_rec;
    bool hitAnything = false;
    auto closest_so_far = t_max;

    for (const auto& object : objects) {
        if (object->hit(r, t_min, closest_so_far, temp_rec)) {
            hitAnything = true;
            closest_so_far = temp_rec.t;
            rec = temp_rec;
        }
    }

    return hitAnything;
}

bool HittableList::boundingBox(double time0, double time1, AABB& output_box) const {
    if (objects.empty()) return false;

    AABB tempBox;
    bool firstBox = true;

    for (const auto& object : objects) {
        if (!object->boundingBox(time0, time1, tempBox)) return false;
        output_box = firstBox ? tempBox : surroundingBox(output_box, tempBox);
        firstBox = false;
    }

    return true;
}