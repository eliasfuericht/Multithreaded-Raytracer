#include <iostream>
#include <fstream>
#include <sstream>

#define STB_IMAGE_IMPLEMENTATION
#define STBI_MSC_SECURE_CRT
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image.h"
#include "stb_image_write.h"

#include "color.h"
#include "Vec3.h"
#include "ray.h"

Color rayColor(const Ray& r) {
    Vec3 unitDirection = normalize(r.getDirection());
    auto t = 0.5 * (unitDirection.getY() + 1.0);
    return (1.0 - t) * Color(1.0, 1.0, 1.0) + t * Color(0.5, 0.7, 1.0);
}

int main() {
    //Image
    const auto aspect = 16.0 / 9.0;
    const int imageWidth = 400;
    const int imageHeight = static_cast<int>(imageWidth / aspect);
    #define CHANNEL_NUM 3

    //Camera 
    auto viewportHeight = 2.0;
    auto viewportWidth = viewportHeight * aspect;
    auto focalLength = 1.0;

    auto origin = Point3(0, 0, 0);
    auto horizontal = Vec3(viewportWidth, 0, 0);
    auto vertical = Vec3(0, viewportHeight, 0);
    auto lowerLeftCorner = origin - horizontal / 2 - vertical / 2 - Vec3(0, 0, focalLength);

    //pixelarray for jpg-output
    uint8_t* pixels = new uint8_t[imageWidth * imageHeight * CHANNEL_NUM];

    //Render
    int index = 0;
    for (int j = imageHeight - 1; j >= 0; --j) {
        std::cerr << "\rScanlines remaining: " << j << ' ' << std::flush;
        for (int i = 0; i < imageWidth; ++i) {

            auto u = double(i) / (imageWidth - 1);
            auto v = double(j) / (imageHeight - 1);
            Ray r(origin, lowerLeftCorner + u * horizontal + v * vertical - origin);
            Color pixelColor = rayColor(r);

            pixels[index++] = static_cast<int>(255.999 * pixelColor.getX());
            pixels[index++] = static_cast<int>(255.999 * pixelColor.getY());
            pixels[index++] = static_cast<int>(255.999 * pixelColor.getZ());
            
            //write_color(std::cout, pixelColor);
        }
    }

    stbi_write_jpg("testImage.jpg", imageWidth, imageHeight, 3, pixels, 100);
    delete[] pixels;

    std::cerr << "\nDone\n";
}