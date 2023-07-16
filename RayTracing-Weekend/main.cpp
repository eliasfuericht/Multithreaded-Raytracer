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

double hitSphere(const Point3& center, double radius, const Ray& r) {
    Vec3 oc = r.getOrigin() - center;
    auto a = r.getDirection().lengthSquared();
    auto halfB = dot(oc, r.getDirection());
    auto c = oc.lengthSquared() - radius * radius;
    auto discriminant = halfB* halfB - a * c;
    if (discriminant < 0) {
        return -1.0;
    }
    else {
        return (-halfB - sqrt(discriminant)) / a;
    }
}

Color rayColor(const Ray& r) {
    auto t = hitSphere(Point3(0, 0, -1), 0.5, r);
    if (t > 0.0) {
        Vec3 N = normalize(r.at(t) - Vec3(0, 0, -1));
        return 0.5 * Color(N.getX() + 1, N.getY() + 1, N.getZ() + 1);
    }
    Vec3 unitDirection = normalize(r.getDirection());
    t = 0.5 * (unitDirection.getY() + 1.0);
    return (1.0 - t) * Color(1.0, 1.0, 1.0) + t * Color(0.5, 0.7, 1.0);
}

int main() {
    //Image
    const auto aspect = 16.0 / 9.0;
    const int imageWidth = 1280;
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
        /*std::cerr << "\rScanlines remaining: " << j << ' ' << std::flush;*/
        for (int i = 0; i < imageWidth; ++i) {

            auto u = double(i) / (imageWidth - 1);
            auto v = double(j) / (imageHeight - 1);
            Ray r(origin, lowerLeftCorner + u * horizontal + v * vertical - origin);
            Color pixelColor = rayColor(r);

            pixels[index++] = static_cast<int>(255.999 * pixelColor.getX());
            pixels[index++] = static_cast<int>(255.999 * pixelColor.getY());
            pixels[index++] = static_cast<int>(255.999 * pixelColor.getZ());
        }
    }

    stbi_write_jpg("testImage.jpg", imageWidth, imageHeight, 3, pixels, 100);
    delete[] pixels;

    std::cerr << "\nPicture rendered!\n";
}