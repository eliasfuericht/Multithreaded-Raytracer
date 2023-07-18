#pragma once

#include "Utility.h"
#include "Hittable.h"

class Material {
public:
	virtual bool scatter(const Ray& rIn, const HitRecord& rec, Color& attentuation, Ray& scattered) const = 0;
};

class Lambertian : public Material {
public:
	Lambertian(const Color& a) {
		albedo = a;
	}

	virtual bool scatter(
		const Ray& rIn, const HitRecord& rec, Color& attenuation, Ray& scattered
	) const override {
		auto scatterDirection = rec.normal + randomInUnitVector();
		if (scatterDirection.nearZero()) scatterDirection = rec.normal;
		scattered = Ray(rec.p, scatterDirection);
		attenuation = albedo;
		return true;
	}

public:
	Color albedo;
};

class Metal : public Material {
public:
	Metal(const Color& a, double f) {
		albedo = a;
		fuzz = f < 1 ? f : 1;
	}

	virtual bool scatter(
		const Ray& rIn, const HitRecord& rec, Color& attenuation, Ray& scattered
	) const override {
		Vec3 reflected = reflect(normalize(rIn.getDirection()), rec.normal);
		scattered = Ray(rec.p, reflected + fuzz * randomInUnitSphere());
		attenuation = albedo;
		return (dot(scattered.getDirection(), rec.normal) > 0);
	}

public:
	Color albedo;
	double fuzz;
};