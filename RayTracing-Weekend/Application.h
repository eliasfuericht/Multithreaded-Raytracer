#pragma once

#define STB_IMAGE_IMPLEMENTATION
#define STBI_MSC_SECURE_CRT
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image.h"
#include "stb_image_write.h"

#include "GUI.h"
#include "Sphere.h"
#include "Renderer.h"
#include "MovingSphere.h"

class Application {
public:
	Application();
	Application(int w, int h);

	void init();
	void run();

	HittableList random_scene();

public:
	std::unique_ptr<std::thread> guiThread;
	HittableList world;
	uint8_t* pixels;

	int windowWidth, windowHeight;
	bool running = true;
};

Application::Application() 
	: windowWidth(1280), windowHeight(720)
{
}

Application::Application(int w, int h) 
	: windowWidth(w), windowHeight(h)
{
}

void Application::init() {
	guiThread = std::make_unique<std::thread>(&GUI::runGUI, windowWidth, windowHeight); // Create a new thread
	std::unique_lock<std::mutex> lock(GUI::cvMutex);
	GUI::cv.wait(lock, [] { return GUI::startRender; });
	world = random_scene(); // Use the member variable 'world', not a local variable 'world'
}

void Application::run() {
	while (running) {
		if (GUI::startRender) {
			pixels = GUI::getRenderer()->render(world, *GUI::getCamera());
			stbi_write_jpg("currentRender.jpg", GUI::getRenderer()->imageWidth, GUI::getRenderer()->imageHeight, 3, pixels, 100);
			delete[] pixels;
			std::cerr << "Picture written to disk!\n";
			GUI::startRender = false;
		}
	}
	guiThread->join();
}

HittableList Application::random_scene() {
	HittableList world;

	auto ground_material = make_shared<Lambertian>(Color(0.5, 0.5, 0.5));
	world.add(make_shared<Sphere>(Point3(0, -1000, 0), 1000, ground_material));

	for (int a = -11; a < 11; a++) {
		for (int b = -11; b < 11; b++) {
			auto choose_mat = randomD();
			Point3 center(a + 0.9 * randomD(), 0.2, b + 0.9 * randomD());
			if ((center - Point3(4, 0.2, 0)).length() > 0.9) {
				shared_ptr<Material> sphere_material;
				if (choose_mat < 0.8) {
					// diffuse
					auto albedo = Color::random() * Color::random();
					sphere_material = make_shared<Lambertian>(albedo);
					auto center2 = center + Vec3(0, randomD(0, .5), 0);
					world.add(make_shared<MovingSphere>(
						center, center2, 0.0, 1.0, 0.2, sphere_material));
				}
				else if (choose_mat < 0.95) {
					// metal
					auto albedo = Color::random(0.5, 1);
					auto fuzz = randomD(0, 0.5);
					sphere_material = make_shared<Metal>(albedo, fuzz);
					world.add(make_shared<Sphere>(center, 0.2, sphere_material));
				}
				else {
					// glass
					sphere_material = make_shared<Dielectric>(1.5);
					world.add(make_shared<Sphere>(center, 0.2, sphere_material));
				}
			}
		}
	}


	auto material1 = make_shared<Dielectric>(1.5);
	world.add(make_shared<Sphere>(Point3(0, 1, 0), 1.0, material1));

	auto material2 = make_shared<Lambertian>(Color(0.0, 0.5, 0.5));
	world.add(make_shared<Sphere>(Point3(-4, 1, 0), 1.0, material2));

	auto material3 = make_shared<Metal>(Color(0.7, 0.6, 0.5), 0.0);
	world.add(make_shared<Sphere>(Point3(4, 1, 0), 1.0, material3));

	return world;
}
