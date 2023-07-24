#include <iostream>
#include <fstream>
#include <sstream>
#include <windows.h>
#include <chrono>
#include <thread>
#include <atomic>

#include "vendor/imgui/imgui.h"
#include "vendor/imgui/imgui_impl_sdl2.h"
#include "vendor/imgui/imgui_impl_opengl3.h"
#include <stdio.h>
#include <SDL.h>
#if defined(IMGUI_IMPL_OPENGL_ES2)
#include <SDL_opengles2.h>
#else
#include <SDL_opengl.h>
#endif

//jpg output
#define STB_IMAGE_IMPLEMENTATION
#define STBI_MSC_SECURE_CRT
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image.h"
#include "stb_image_write.h"

//own includes
#include "Utility.h"
#include "Camera.h"
#include "Color.h"
#include "HittableList.h"
#include "Material.h"
#include "Sphere.h"
#include "Renderer.h" 

double hitSphere(const Point3& center, double radius, const Ray& r) {
	Vec3 oc = r.getOrigin() - center;
	auto a = r.getDirection().lengthSquared();
	auto halfB = dot(oc, r.getDirection());
	auto c = oc.lengthSquared() - radius * radius;
	auto discriminant = halfB * halfB - a * c;
	if (discriminant < 0) {
		return -1.0;
	}
	else {
		return (-halfB - sqrt(discriminant)) / a;
	}
}

Color rayColor(const Ray& r, const Hittable& w, int depth) {
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

HittableList random_scene() {
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
					world.add(make_shared<Sphere>(center, 0.2, sphere_material));
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

void runGUI() {
	if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER | SDL_INIT_GAMECONTROLLER) != 0)
	{
		printf("Error: %s\n", SDL_GetError());
		return;
	}

	// GL 3.0 + GLSL 130
	const char* glsl_version = "#version 130";
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, 0);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);

#ifdef SDL_HINT_IME_SHOW_UI
	SDL_SetHint(SDL_HINT_IME_SHOW_UI, "1");
#endif

	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
	SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
	SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);
	SDL_WindowFlags window_flags = (SDL_WindowFlags)(SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALLOW_HIGHDPI);
	SDL_Window* window = SDL_CreateWindow("Dear ImGui SDL2+OpenGL3 example", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 1280, 720, window_flags);
	SDL_GLContext gl_context = SDL_GL_CreateContext(window);
	SDL_GL_MakeCurrent(window, gl_context);
	SDL_GL_SetSwapInterval(1);

	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls

	// Setup Dear ImGui style
	ImGui::StyleColorsDark();
	//ImGui::StyleColorsLight();

	// Setup Platform/Renderer backends
	ImGui_ImplSDL2_InitForOpenGL(window, gl_context);
	ImGui_ImplOpenGL3_Init(glsl_version);

	// Our state
	bool show_demo_window = true;
	bool show_another_window = false;
	ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);


	bool done = false;
#ifdef __EMSCRIPTEN__
	// For an Emscripten build we are disabling file-system access, so let's not attempt to do a fopen() of the imgui.ini file.
	// You may manually call LoadIniSettingsFromMemory() to load settings from your own storage.
	io.IniFilename = nullptr;
	EMSCRIPTEN_MAINLOOP_BEGIN
#else
	while (!done)
#endif
	{
		// Poll and handle events (inputs, window resize, etc.)
		// You can read the io.WantCaptureMouse, io.WantCaptureKeyboard flags to tell if dear imgui wants to use your inputs.
		// - When io.WantCaptureMouse is true, do not dispatch mouse input data to your main application, or clear/overwrite your copy of the mouse data.
		// - When io.WantCaptureKeyboard is true, do not dispatch keyboard input data to your main application, or clear/overwrite your copy of the keyboard data.
		// Generally you may always pass all inputs to dear imgui, and hide them from your application based on those two flags.
		SDL_Event event;
		while (SDL_PollEvent(&event))
		{
			ImGui_ImplSDL2_ProcessEvent(&event);
			if (event.type == SDL_QUIT)
				done = true;
			if (event.type == SDL_WINDOWEVENT && event.window.event == SDL_WINDOWEVENT_CLOSE && event.window.windowID == SDL_GetWindowID(window))
				done = true;
		}

		// Start the Dear ImGui frame
		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplSDL2_NewFrame();
		ImGui::NewFrame();

		// 1. Show the big demo window (Most of the sample code is in ImGui::ShowDemoWindow()! You can browse its code to learn more about Dear ImGui!).
		if (show_demo_window)
			ImGui::ShowDemoWindow(&show_demo_window);

		// 2. Show a simple window that we create ourselves. We use a Begin/End pair to create a named window.
		{
			static float f = 0.0f;
			static int counter = 0;

			ImGui::Begin("Hello, world!");                          // Create a window called "Hello, world!" and append into it.

			ImGui::Text("This is some useful text.");               // Display some text (you can use a format strings too)
			ImGui::Checkbox("Demo Window", &show_demo_window);      // Edit bools storing our window open/close state
			ImGui::Checkbox("Another Window", &show_another_window);

			ImGui::SliderFloat("float", &f, 0.0f, 1.0f);            // Edit 1 float using a slider from 0.0f to 1.0f
			ImGui::ColorEdit3("clear color", (float*)&clear_color); // Edit 3 floats representing a color

			if (ImGui::Button("Button"))                            // Buttons return true when clicked (most widgets return true when edited/activated)
				counter++;
			ImGui::SameLine();
			ImGui::Text("counter = %d", counter);

			ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / io.Framerate, io.Framerate);
			ImGui::End();

		}

		// 3. Show another simple window.
		if (show_another_window)
		{
			ImGui::Begin("Another Window", &show_another_window);   // Pass a pointer to our bool variable (the window will have a closing button that will clear the bool when clicked)
			ImGui::Text("Hello from another window!");
			if (ImGui::Button("Close Me"))
				show_another_window = false;
			ImGui::End();
		}

		// Rendering
		ImGui::Render();
		glViewport(0, 0, (int)io.DisplaySize.x, (int)io.DisplaySize.y);
		glClearColor(clear_color.x * clear_color.w, clear_color.y * clear_color.w, clear_color.z * clear_color.w, clear_color.w);
		glClear(GL_COLOR_BUFFER_BIT);
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
		SDL_GL_SwapWindow(window);
	}
#ifdef __EMSCRIPTEN__
	EMSCRIPTEN_MAINLOOP_END;
#endif

	// Cleanup
	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplSDL2_Shutdown();
	ImGui::DestroyContext();

	SDL_GL_DeleteContext(gl_context);
	SDL_DestroyWindow(window);
	SDL_Quit();
}

int main(int argc, char* args[]) {

	std::thread guiThread(runGUI);
	
	//Time start
	auto start = std::chrono::high_resolution_clock::now();

	//Image
	const auto aspect = 16.0 / 9.0;
	const int imageWidth = 1080;
	const int imageHeight = static_cast<int>(imageWidth / aspect);
	const int samplesPerPixel = 10;
	const int maxDepth = 10;
	const int channelNumber = 3;

	//World
	auto world = random_scene();

	Point3 lookfrom(13, 2, 3);
	Point3 lookat(0, 0, 0);
	Vec3 vup(0, 1, 0);
	auto dist_to_focus = 10.0;
	auto aperture = 0.1;
	Camera camera(lookfrom, lookat, vup, 20, aspect, aperture, dist_to_focus);

	//pixelarray for jpg-output
	uint8_t* pixels = new uint8_t[imageWidth * imageHeight * channelNumber];

	int numThreads = std::thread::hardware_concurrency()-2;
	
	int stripHeight = imageHeight / numThreads;

	std::vector<std::thread> threads;

	std::vector<uint8_t*> threadPixels(numThreads);

	for (int t = 0; t < numThreads; ++t) {
		int startY = t * stripHeight;
		int endY = (t + 1) * stripHeight;

		if (t == numThreads - 1) {
			endY = imageHeight;
		}

		threadPixels[t] = new uint8_t[imageWidth * (endY - startY) * channelNumber];

		int tracker = imageHeight;
		threads.emplace_back([startY, endY, &world, &camera, imageWidth, imageHeight, channelNumber, samplesPerPixel, maxDepth, threadPixels, t, &tracker]() {
			for (int j = endY - 1; j >= startY; --j) {
				int scanlineIndex = 0;
				uint8_t* scanlinePixels = &threadPixels[t][(endY - 1 - j) * imageWidth * channelNumber];
				for (int i = 0; i < imageWidth; ++i) {
					Color pixelColor(0, 0, 0);
					for (int s = 0; s < samplesPerPixel; ++s) {
						auto u = (i + randomD()) / (imageWidth - 1);
						auto v = (j + randomD()) / (imageHeight - 1);
						Ray r = camera.getRay(u, v);
						pixelColor += rayColor(r, world, maxDepth);
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
				std::cerr << "\rScanlines Left: " << tracker << " ";
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

	auto end = std::chrono::high_resolution_clock::now();
	std::chrono::duration<double> duration = end - start;
	double seconds = duration.count();
	std::cerr << "\nPicture rendered!\nIt took: " << seconds << " seconds\n";

	stbi_write_jpg("testImage.jpg", imageWidth, imageHeight, 3, pixels, 100);
	delete[] pixels;
	ShellExecuteA(NULL, "open", "testImage.jpg", NULL, NULL, SW_SHOWNORMAL);

	std::cerr << "Picture written to disk!\n";

	return 0;
}