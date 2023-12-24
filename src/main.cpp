#include <memory>
#include "pch.h"

#include "camera.h"
#include "hittable_list.h"
#include "material.h"
#include "sphere.h"

int main(const int argc, const char* argv[]) {
    rt::hittable_list world;
    auto ground_material =
        std::make_shared<rt::lambertian>(rt::color(0.5, 0.5, 0.5));
    world.add(std::make_shared<rt::sphere>(rt::point3(0, -1000, 0), 1000,
                                           ground_material));

    for (int a = -11; a < 11; a++) {
        for (int b = -11; b < 11; b++) {
            auto choose_mat = rt::random_double();
            rt::point3 center(a + 0.9 * rt::random_double(), 0.2,
                              b + 0.9 * rt::random_double());

            if ((center - rt::point3(4, 0.2, 0)).length() > 0.9) {
                std::shared_ptr<rt::material> sphere_material;

                if (choose_mat < 0.8) {
                    // diffuse
                    auto albedo = rt::color::random() * rt::color::random();
                    sphere_material = std::make_shared<rt::lambertian>(albedo);
                    world.add(std::make_shared<rt::sphere>(center, 0.2,
                                                           sphere_material));
                } else if (choose_mat < 0.95) {
                    // metal
                    auto albedo = rt::color::random(0.5, 1);
                    auto fuzz = rt::random_double(0, 0.5);
                    sphere_material = std::make_shared<rt::metal>(albedo, fuzz);
                    world.add(std::make_shared<rt::sphere>(center, 0.2,
                                                           sphere_material));
                } else {
                    // glass
                    sphere_material = std::make_shared<rt::dielectric>(1.5);
                    world.add(std::make_shared<rt::sphere>(center, 0.2,
                                                           sphere_material));
                }
            }
        }
    }

    auto material1 = std::make_shared<rt::dielectric>(1.5);
    world.add(
        std::make_shared<rt::sphere>(rt::point3(0, 1, 0), 1.0, material1));

    auto material2 = std::make_shared<rt::lambertian>(rt::color(0.4, 0.2, 0.1));
    world.add(
        std::make_shared<rt::sphere>(rt::point3(-4, 1, 0), 1.0, material2));

    auto material3 = std::make_shared<rt::metal>(rt::color(0.7, 0.6, 0.5), 0.0);
    world.add(
        std::make_shared<rt::sphere>(rt::point3(4, 1, 0), 1.0, material3));

    rt::camera cam;

    cam.aspect_ratio = 16.0 / 9.0;
    cam.image_width = 1200;
    cam.samples_per_pixel = 100;
    cam.max_depth = 50;

    cam.vfov = 20;
    cam.lookfrom = rt::point3(13, 2, 3);
    cam.lookat = rt::point3(0, 0, 0);
    cam.vup = rt::vec3(0, 1, 0);

    cam.defocus_angle = 0.6;
    cam.focus_dist = 10.0;

    cam.render(world, 6);

    return 0;
}
