#pragma once
#include <iostream>
#include <fstream>
#include <sstream>
#include <windows.h>
#include <chrono>
#include <thread>
#include <atomic>

#include "Application.h"

int main(int argc, char* args[]) {

	Application app(1280, 720);
	app.init();
	app.run();

	return 0;
}