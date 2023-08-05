#pragma once

#define STB_IMAGE_IMPLEMENTATION
#define STBI_MSC_SECURE_CRT
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image.h"
#include "stb_image_write.h"

#include "GUI.h"
#include "Sphere.h"
#include "Renderer.h"

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

	auto material1 = make_shared<Dielectric>(1.5);
	world.add(make_shared<Sphere>(Point3(0, 1, 0), 1.0, material1));

	auto material2 = make_shared<Lambertian>(Color(0.0, 0.5, 0.5));
	world.add(make_shared<Sphere>(Point3(-4, 1, 0), 1.0, material2));

	auto material3 = make_shared<Metal>(Color(0.7, 0.6, 0.5), 0.0);
	world.add(make_shared<Sphere>(Point3(4, 1, 0), 1.0, material3));

	return world;
}
