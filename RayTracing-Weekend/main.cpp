#pragma once
#include <iostream>
#include <fstream>
#include <sstream>
#include <windows.h>
#include <chrono>
#include <thread>
#include <atomic>

//jpg output
#define STB_IMAGE_IMPLEMENTATION
#define STBI_MSC_SECURE_CRT
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image.h"
#include "stb_image_write.h"

//own includes
#include "Renderer.h"
#include "Sphere.h"
#include "GUI.h"

HittableList random_scene() {
	HittableList world;

	auto ground_material = make_shared<Lambertian>(Color(0.5, 0.5, 0.5));
	world.add(make_shared<Sphere>(Point3(0, -1000, 0), 1000, ground_material));

	//for (int a = -11; a < 11; a++) {
	//	for (int b = -11; b < 11; b++) {
	//		auto choose_mat = randomD();
	//		Point3 center(a + 0.9 * randomD(), 0.2, b + 0.9 * randomD());

	//		if ((center - Point3(4, 0.2, 0)).length() > 0.9) {
	//			shared_ptr<Material> sphere_material;

	//			if (choose_mat < 0.8) {
	//				// diffuse
	//				auto albedo = Color::random() * Color::random();
	//				sphere_material = make_shared<Lambertian>(albedo);
	//				world.add(make_shared<Sphere>(center, 0.2, sphere_material));
	//			}
	//			else if (choose_mat < 0.95) {
	//				// metal
	//				auto albedo = Color::random(0.5, 1);
	//				auto fuzz = randomD(0, 0.5);
	//				sphere_material = make_shared<Metal>(albedo, fuzz);
	//				world.add(make_shared<Sphere>(center, 0.2, sphere_material));
	//			}
	//			else {
	//				// glass
	//				sphere_material = make_shared<Dielectric>(1.5);
	//				world.add(make_shared<Sphere>(center, 0.2, sphere_material));
	//			}
	//		}
	//	}
	//}

	auto material1 = make_shared<Dielectric>(1.5);
	world.add(make_shared<Sphere>(Point3(0, 1, 0), 1.0, material1));

	auto material2 = make_shared<Lambertian>(Color(0.0, 0.5, 0.5));
	world.add(make_shared<Sphere>(Point3(-4, 1, 0), 1.0, material2));

	auto material3 = make_shared<Metal>(Color(0.7, 0.6, 0.5), 0.0);
	world.add(make_shared<Sphere>(Point3(4, 1, 0), 1.0, material3));

	return world;
}

int main(int argc, char* args[]) {

	//Time start
	auto start = std::chrono::high_resolution_clock::now();

	//Image & renderer
	Renderer renderer = Renderer(1680, 1, 1, true);

	std::thread guiThread(GUI::runGUI,1920,1080,&renderer);

	HittableList world = random_scene();

	Camera camera(Point3(25, 6, 8), Point3(0, 0, 0), Vec3(0, 1, 0), 20, renderer.aspect, 0.1, 20);

	uint8_t* pixels = renderer.render(world, camera);

	auto end = std::chrono::high_resolution_clock::now();
	std::chrono::duration<double> duration = end - start;
	double seconds = duration.count();
	std::cerr << "\nPicture rendered!\nIt took: " << seconds << " seconds\n";

	stbi_write_jpg("currentRender.jpg", renderer.imageWidth, renderer.imageHeight, 3, pixels, 100);
	delete[] pixels;
	GUI::toggleRendering();
	//ShellExecuteA(NULL, "open", "currentRender.jpg", NULL, NULL, SW_SHOWNORMAL);

	std::cerr << "Picture written to disk!\n";

	guiThread.join();

	return 0;
}