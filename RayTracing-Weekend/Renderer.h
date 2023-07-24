#pragma once
class Renderer
{
public:
	Renderer(int width, int height, int samples, int bounces, bool multithread);

	int imageWidth;
	int imageHeight;
	int samplesPerPixel;
	int bounces;
	float aspect = imageWidth / imageHeight;
	const int channelNumber = 3;
	bool multithreaded;
};

