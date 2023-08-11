#pragma once

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

#include "Renderer.h"

namespace GUI {
	void runGUI(int windowW, int windowH);

	bool loadTextureFromFile(const char* filename, GLuint* out_texture, int* out_width, int* out_height);
	Renderer* getRenderer();
	Camera* getCamera();

	Renderer* renderer = new Renderer();
	Camera* camera;

	std::condition_variable cv;
	std::mutex cvMutex;

	float lookFrom[] = { 10.0f,10.0f,10.0f };
	float lookAt[] = { 0.0f,0.0f,0.0f };
	float vUp[] = { 0.0f,1.0f,0.0f };
	float fov = 45.0f;
	float aspectRatio = 1.7777f;
	float aperture = 0.1f;
	float focusDistance = 20.0f;
}

void GUI::runGUI(int windowW, int windowH) {
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
		SDL_Window* window = SDL_CreateWindow("RayTracer", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, windowW, windowH, window_flags);
		SDL_GLContext gl_context = SDL_GL_CreateContext(window);
		SDL_GL_MakeCurrent(window, gl_context);
		SDL_GL_SetSwapInterval(1);

		IMGUI_CHECKVERSION();
		ImGui::CreateContext();
		ImGuiIO& io = ImGui::GetIO(); (void)io;
		io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
		io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
		io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;         // Enable Docking
		io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;       // Enable Multi-Viewport / Platform Windows

		ImGui::StyleColorsDark();

		ImGuiStyle& style = ImGui::GetStyle();
		if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
		{
			style.WindowRounding = 0.0f;
			style.Colors[ImGuiCol_WindowBg].w = 1.0f;
		}

		ImGui_ImplSDL2_InitForOpenGL(window, gl_context);
		ImGui_ImplOpenGL3_Init(glsl_version);

		ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

		bool done = false;
#ifdef __EMSCRIPTEN__
		io.IniFilename = nullptr;
		EMSCRIPTEN_MAINLOOP_BEGIN
#else
		while (!done)
#endif
		{
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

			//Demo Window
			ImGui::ShowDemoWindow();
			
			//Render Settings Window
			{
				ImGui::Begin("Render Settings");
				ImGui::SliderInt("Image Width", &renderer->imageWidth, 10, 2000);
				ImGui::SliderInt("Image Height", &renderer->imageHeight, 10, 2000);
				ImGui::SliderInt("Samples", &renderer->samplesPerPixel, 1, 50);
				ImGui::SliderInt("Bounces", &renderer->depth, 1, 50);
				ImGui::Checkbox("multithreading", &renderer->multithreaded);
				
				ImGui::SliderFloat3("Camera Position", lookFrom, 0.1f, 20.0f);
				ImGui::SliderFloat3("Camera Lookat", lookAt, 0.0f, 20.0f);
				ImGui::SliderFloat("Camera FOV", &fov, 1.0f, 90.0f);
				ImGui::SliderFloat("Camera Aperture", &aperture, 0.01f, 1.0f);
				ImGui::SliderFloat("Camera Focus Distance", &focusDistance, 1.0f, 50.0f);
				
				if (ImGui::Button("Start Rendering")) {
					GUI::renderer->recalculateImageSize();
					GUI::camera = new Camera(Point3(lookFrom[0], lookFrom[1], lookFrom[2]), Point3(lookAt[0], lookAt[1], lookAt[2]), 
													Vec3(vUp[0], vUp[1], vUp[2]), (double)fov, (double)aspectRatio, (double)aperture, (double)focusDistance, 0.0, 1.0);
					GUI::renderer->renderInfo.rendering = true;
					GUI::cv.notify_one();
				}
				if (ImGui::Button("Stop Rendering")) {
					GUI::renderer->renderInfo.rendering = false;
				}
				ImGui::End();
			}

			//Progress Window
			{
				int f = (int)GUI::renderer->renderInfo.progress;
				float progress = 100 - ((float)GUI::renderer->renderInfo.progress / GUI::renderer->imageHeight) * 100.0f;
				ImGui::Begin("Raytracer Progress");
				ImGui::SliderInt("Scanlines remaining", &f, 0, GUI::renderer->imageHeight);
				ImGui::SliderFloat("Progress:", &progress, 0.0f, 100.0f);
				ImGui::Text("Rendertime: %.2fseconds", GUI::renderer->renderInfo.time);
				//ImGui::SliderFloat("Rendertime:", &GUI::renderer->renderInfo.time, 0.0f, 100.0f);
				ImGui::End();
			}

			//output Window
			{
				ImGui::Begin("Output");
				if (!GUI::renderer->renderInfo.rendering)
				{
					int my_image_width = 0;
					int my_image_height = 0;
					GLuint my_image_texture = 0;
					bool ret = GUI::loadTextureFromFile("currentRender.jpg", &my_image_texture, &my_image_width, &my_image_height);
					ImGui::Image((void*)(intptr_t)my_image_texture, ImVec2((float)my_image_width, (float)my_image_height));
				}
				ImGui::End();
			}

			// Rendering
			ImGui::Render();
			glViewport(0, 0, (int)io.DisplaySize.x, (int)io.DisplaySize.y);
			glClearColor(clear_color.x * clear_color.w, clear_color.y * clear_color.w, clear_color.z * clear_color.w, clear_color.w);
			glClear(GL_COLOR_BUFFER_BIT);
			ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

			if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
			{
				SDL_Window* backup_current_window = SDL_GL_GetCurrentWindow();
				SDL_GLContext backup_current_context = SDL_GL_GetCurrentContext();
				ImGui::UpdatePlatformWindows();
				ImGui::RenderPlatformWindowsDefault();
				SDL_GL_MakeCurrent(backup_current_window, backup_current_context);
			}

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

bool GUI::loadTextureFromFile(const char* filename, GLuint* out_texture, int* out_width, int* out_height)
{
	// Load from file
	int image_width = 0;
	int image_height = 0;
	unsigned char* image_data = stbi_load(filename, &image_width, &image_height, NULL, 4);
	if (image_data == NULL)
		return false;

	// Create a OpenGL texture identifier
	GLuint image_texture;
	glGenTextures(1, &image_texture);
	glBindTexture(GL_TEXTURE_2D, image_texture);

	// Setup filtering parameters for display
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE); // This is required on WebGL for non power-of-two textures
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE); // Same

	// Upload pixels into texture
#if defined(GL_UNPACK_ROW_LENGTH) && !defined(__EMSCRIPTEN__)
	glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
#endif
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, image_width, image_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, image_data);
	stbi_image_free(image_data);

	*out_texture = image_texture;
	*out_width = image_width;
	*out_height = image_height;

	return true;
}

Renderer* GUI::getRenderer()
{
	return GUI::renderer;
}

Camera* GUI::getCamera()
{
	return GUI::camera;
}
