#pragma once

#include "Utility.h"

class Camera {
public:

	Camera() {
		Point3 lookfrom = Point3(10, 5, 8);
		Point3 lookat = Point3(0, 0, 0);
		Vec3 vup = Vec3(0, 1, 0);
		double vfov = 20;
		double aspectRatio = 1.7777;
		double aperture = 0.1;
		double focusDistance = 20;
		auto theta = degToRad(vfov);
		auto h = tan(theta / 2);
		auto viewportHeight = 2.0 * h;
		auto viewportWidth = aspectRatio * viewportHeight;
		w = normalize(lookfrom - lookat);
		u = normalize(cross(vup, w));
		v = cross(w, u);

		auto w = normalize(lookfrom - lookat);
		auto u = normalize(cross(vup, w));
		auto v = cross(w, u);

		origin = lookfrom;
		horizontal = focusDistance * viewportWidth * u;
		vertical = focusDistance * viewportHeight * v;
		lowerLeftCorner = origin - horizontal / 2 - vertical / 2 - focusDistance * w;

		lens_radius = aperture / 2;
	}
	Camera(Point3 lookfrom, Point3 lookat, Vec3 vup, double vfov, double aspectRatio, double aperture, double focusDistance) {
		auto theta = degToRad(vfov);
		auto h = tan(theta / 2);
		auto viewportHeight = 2.0 * h;
		auto viewportWidth = aspectRatio * viewportHeight;
		w = normalize(lookfrom - lookat);
		u = normalize(cross(vup, w));
		v = cross(w, u);

		auto w = normalize(lookfrom - lookat);
		auto u = normalize(cross(vup, w));
		auto v = cross(w, u);

		origin = lookfrom;
		horizontal = focusDistance * viewportWidth * u;
		vertical = focusDistance * viewportHeight * v;
		lowerLeftCorner = origin - horizontal / 2 - vertical / 2 - focusDistance * w;

		lens_radius = aperture / 2;
	};

	Ray getRay(double s, double t) const {
		Vec3 rd = lens_radius * randomInUnitDisk();
		Vec3 offset = u * rd.getX() + v * rd.getY();

		return Ray(
			origin + offset,
			lowerLeftCorner + s * horizontal + t * vertical - origin - offset
		);
	}

public:
	Point3 origin;
	Point3 lowerLeftCorner;
	Vec3 horizontal;
	Vec3 vertical;
	Vec3 u, v, w;
	double lens_radius;
};

