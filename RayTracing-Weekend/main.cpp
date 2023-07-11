#include <iostream>
#include <fstream>
#include <sstream>

#include "color.h"

#define STB_IMAGE_IMPLEMENTATION
#define STBI_MSC_SECURE_CRT
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image.h"
#include "stb_image_write.h"

int main() {
	const int image_width = 256;
	const int image_height = 256;
    #define CHANNEL_NUM 3

    uint8_t* pixels = new uint8_t[image_width * image_height * CHANNEL_NUM];
    vec3 vec = vec3(1, 1, 1);

    vec = vec + vec;

    std::cout << vec << std::endl;

    int index = 0;
    for (int j = image_height - 1; j >= 0; --j) {
        std::cerr << "\rScanlines remaining: " << j << ' ' << std::flush;
        for (int i = 0; i < image_width; ++i) {

            color pixel_color(double(i) / (image_width - 1), double(j) / (image_height - 1), 0.25);

            pixels[index++] = static_cast<int>(255.999 * pixel_color.x());
            pixels[index++] = static_cast<int>(255.999 * pixel_color.y());
            pixels[index++] = static_cast<int>(255.999 * pixel_color.z());
            
            //write_color(std::cout, pixel_color);
        }
    }

    stbi_write_jpg("testImage.jpg", image_width, image_height, 3, pixels, 100);
    delete[] pixels;

    std::cerr << "\nDone\n";
}