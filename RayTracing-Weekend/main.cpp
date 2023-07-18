#include <iostream>
#include <fstream>
#include <sstream>
#include <windows.h>
#include <chrono>
#include <thread>

//jpg output
#define STB_IMAGE_IMPLEMENTATION
#define STBI_MSC_SECURE_CRT
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image.h"
#include "stb_image_write.h"

//own utilities
#include "Utility.h"

#include "Camera.h"
#include "Color.h"
#include "HittableList.h"
#include "Material.h"
#include "Sphere.h"

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

    auto material2 = make_shared<Lambertian>(Color(0.4, 0.2, 0.1));
    world.add(make_shared<Sphere>(Point3(-4, 1, 0), 1.0, material2));

    auto material3 = make_shared<Metal>(Color(0.7, 0.6, 0.5), 0.0);
    world.add(make_shared<Sphere>(Point3(4, 1, 0), 1.0, material3));

    return world;
}

int main() {
    //Image
    const auto aspect = 3.0 / 2.0;
    const int imageWidth = 1200;
    const int imageHeight = static_cast<int>(imageWidth / aspect);
    const int samplesPerPixel = 8;
    const int maxDepth = 50;
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

    //Time start
    auto start = std::chrono::high_resolution_clock::now();

    //Render
    int index = 0;
    for (int j = imageHeight - 1; j >= 0; --j) {
        //std::cerr << "\rScanlines remaining: " << j << ' ' << std::flush;
        std::cerr << "\rRendering progress: " << (int)((imageHeight - j) * 100.0) / imageHeight << "% " << std::flush;
        for (int i = 0; i < imageWidth; ++i) {
            Color pixelColor(0, 0, 0);
            for (int s = 0; s < samplesPerPixel; ++s) {
                auto u = (i + randomD()) / (imageWidth - 1);
                auto v = (j + randomD()) / (imageHeight - 1);
                Ray r = camera.getRay(u, v);
                //Ray r(camera.origin, camera.lowerLeftCorner + u * camera.horizontal + v * camera.vertical - camera.origin);
                pixelColor += rayColor(r, world,maxDepth);
            }
            auto r = pixelColor.getX();
            auto g = pixelColor.getY();
            auto b = pixelColor.getZ();

            // Divide the color by the number of samples.
            auto scale = 1.0 / samplesPerPixel;
            r = sqrt(scale * r);
            g = sqrt(scale * g);
            b = sqrt(scale * b);

            pixels[index++] = static_cast<int>(256 * clamp(r, 0.0, 0.999));
            pixels[index++] = static_cast<int>(256 * clamp(g, 0.0, 0.999));
            pixels[index++] = static_cast<int>(256 * clamp(b, 0.0, 0.999));
        }
    }

    // Get the current time point after the operation completes
    auto end = std::chrono::high_resolution_clock::now();

    // Calculate the duration of the operation
    std::chrono::duration<double> duration = end - start;
    double seconds = duration.count();

    std::cerr << "\nPicture rendered!\nIt took: " << seconds << " seconds\n";

    stbi_write_jpg("testImage.jpg", imageWidth, imageHeight, 3, pixels, 100);
    delete[] pixels;
    ShellExecuteA(NULL, "open", "testImage.jpg", NULL, NULL, SW_SHOWNORMAL);

    std::cerr << "Picture written to disk!\n";
}