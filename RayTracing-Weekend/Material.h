#pragma once

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
		scattered = Ray(rec.p, scatterDirection, rIn.getTime());
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
		scattered = Ray(rec.p, reflected + fuzz * randomInUnitSphere(), rIn.getTime());
		attenuation = albedo;
		return (dot(scattered.getDirection(), rec.normal) > 0);
	}

public:
	Color albedo;
	double fuzz;
};

class Dielectric : public Material {
public:
	Dielectric(double indexOfRefraction) {
		ir = indexOfRefraction;
	}

	virtual bool scatter(
		const Ray& rIn, const HitRecord& rec, Color& attenuation, Ray& scattered
	) const override {
		attenuation = Color(1.0, 1.0, 1.0);
		double refractionRatio = rec.frontFace ? (1.0 / ir) : ir;

		Vec3 unitDirection = normalize(rIn.getDirection());
		double cosTheta = fmin(dot(-unitDirection, rec.normal), 1.0);
		double sinTheta = sqrt(1.0 - cosTheta * cosTheta);

		bool cannot_refract = refractionRatio * sinTheta > 1.0;
		Vec3 direction;

		if (cannot_refract || reflectance(cosTheta, refractionRatio) > randomD())
			direction = reflect(unitDirection, rec.normal);
		else
			direction = refract(unitDirection, rec.normal, refractionRatio);

		scattered = Ray(rec.p, direction, rIn.getTime());
		return true;
	}
public: 
	double ir; //Intex of Refraction

private:
	static double reflectance(double cosine, double ref_idx) {
		// Use Schlick's approximation for reflectance.
		auto r0 = (1 - ref_idx) / (1 + ref_idx);
		r0 = r0 * r0;
		return r0 + (1 - r0) * pow((1 - cosine), 5);
	}
};