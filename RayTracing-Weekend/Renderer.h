#pragma once

#include <thread>
#include <vector>
#include <mutex>

#include "GUI.h"
#include "Material.h"
#include "HittableList.h"
#include "Camera.h"

struct renderingInformation {
	bool rendering;
	float time;
	float progress;
};

class Renderer
{
public:
    Renderer();
    Renderer(int width, int samples, int bounces, bool multithread);

    Color rayColor(const Ray& r, const Hittable& w, int depth);

    uint8_t* render(HittableList world, Camera& camera);
	uint8_t* getCurrentPixels() { return currentPixels; };

	void recalculateImageSize() { imageHeight = static_cast<int>(imageWidth / aspect); };

	renderingInformation renderInfo;

    int imageWidth;
    int samplesPerPixel;
    int depth;
    bool multithreaded;
    float aspect;
    const int channelNumber = 3;
	int imageHeight;

private:
	uint8_t* currentPixels = nullptr;
};

Renderer::Renderer() :
	imageWidth(640),
	aspect(16.0f / 9.0f),
	imageHeight(static_cast<int>(imageWidth / aspect)),
	samplesPerPixel(8),
	depth(24),
	multithreaded(true)
{
}

Renderer::Renderer(int width, int samples, int bounces, bool multithread) :
	imageWidth(width),
	aspect(16.0f / 9.0f),
	imageHeight(static_cast<int>(imageWidth / aspect)),
	samplesPerPixel(samples),
	depth(bounces),
	multithreaded(multithread)
{
}

Color Renderer::rayColor(const Ray& r, const Hittable& w, int depth)
{
	HitRecord rec;

	// If we've exceeded the ray bounce limit, no more light is gathered.
	if (depth <= 0)
		return Color(0, 0, 0);

	if (w.hit(r, 0.001, infinity, rec)) {
		Ray scattered;
		Color attenuation;
		if (rec.matPtr->scatter(r, rec, attenuation, scattered))
			return attenuation * rayColor(scattered, w, depth - 1);
		return Color(0, 0, 0);
	}
	Vec3 unitDirection = normalize(r.getDirection());
	auto t = 0.5 * (unitDirection.getY() + 1.0);
	return (1.0 - t) * Color(1.0, 1.0, 1.0) + t * Color(0.5, 0.7, 1.0);
}

uint8_t* Renderer::render(HittableList world, Camera& camera) {
	//pixelarray for jpg-output
	uint8_t* pixels = new uint8_t[imageWidth * imageHeight * channelNumber];

	int numThreads;

	if (!multithreaded)
	{
		numThreads = 1;
	}
	else {
		numThreads = std::thread::hardware_concurrency() - 2;
	}

	int stripHeight = imageHeight / numThreads;

	std::vector<std::thread> threads;

	std::vector<uint8_t*> threadPixels(numThreads);

	renderInfo.rendering = true;
	auto start = std::chrono::high_resolution_clock::now();

	for (int t = 0; t < numThreads; ++t) {
		int startY = t * stripHeight;
		int endY = (t + 1) * stripHeight;

		if (t == numThreads - 1) {
			endY = imageHeight;
		}

		threadPixels[t] = new uint8_t[imageWidth * (endY - startY) * channelNumber];

		int tracker = imageHeight;
		threads.emplace_back([this, startY, endY, &world, &camera, threadPixels, t, &tracker]() {
			for (int j = endY - 1; j >= startY; --j) {
				int scanlineIndex = 0;
				uint8_t* scanlinePixels = &threadPixels[t][(endY - 1 - j) * imageWidth * channelNumber];
				for (int i = 0; i < imageWidth; ++i) {
					Color pixelColor(0, 0, 0);
					for (int s = 0; s < samplesPerPixel; ++s) {
						auto u = (i + randomD()) / (imageWidth - 1);
						auto v = (j + randomD()) / (imageHeight - 1);
						Ray r = camera.getRay(u, v);
						pixelColor += rayColor(r, world, depth);
					}
					auto r = pixelColor.getX();
					auto g = pixelColor.getY();
					auto b = pixelColor.getZ();

					// Divide the Color by the number of samples.
					auto scale = 1.0 / samplesPerPixel;
					r = sqrt(scale * r);
					g = sqrt(scale * g);
					b = sqrt(scale * b);

					scanlinePixels[scanlineIndex++] = static_cast<int>(256 * clamp(r, 0.0, 0.999));
					scanlinePixels[scanlineIndex++] = static_cast<int>(256 * clamp(g, 0.0, 0.999));
					scanlinePixels[scanlineIndex++] = static_cast<int>(256 * clamp(b, 0.0, 0.999));
				}
				tracker--;
				renderInfo.progress = (float)tracker;
				
			}
		});
	}

	for (auto& thread : threads) {
		thread.join();
	}

	int index = 0;
	for (int t = 0; t < numThreads; ++t) {
		int startY = t * stripHeight;
		int endY = (t + 1) * stripHeight;
		if (t == numThreads - 1) {
			endY = imageHeight;
		}

		int start = (imageHeight - endY) * imageWidth * channelNumber;
		int end = (imageHeight - startY) * imageWidth * channelNumber;

		std::copy(threadPixels[t], threadPixels[t] + (endY - startY) * imageWidth * channelNumber, pixels + start);
		delete[] threadPixels[t];
	}

	//renderInfo.rendering = false;
	auto end = std::chrono::high_resolution_clock::now();
	std::chrono::duration<double> duration = end - start;
	renderInfo.time = (float)duration.count();
	renderInfo.rendering = false;

	return pixels;
};


