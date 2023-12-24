#pragma once

#include "pch.h"

#include "color.h"
#include "hittable.h"
#include "material.h"

namespace rt {

class camera {
public:
    double aspect_ratio = 1.0;
    int image_width = 100;       // Rendered image width in pixel count
    int samples_per_pixel = 10;  // Count of random samples for each pixel
    int max_depth = 10;          // Maximum number of ray bounces into scene

    double vfov = 90;                    // Vertical view angle (field of view)
    point3 lookfrom = point3(0, 0, -1);  // Point camera is looking from
    point3 lookat = point3(0, 0, 0);     // Point camera is looking at
    vec3 vup = vec3(0, 1, 0);            // Camera-relative "up" direction

    double defocus_angle = 0;  // Variation angle of rays through each pixel
    double focus_dist =
        10;  // Distance from camera lookfrom point to plane of perfect focus

    void render(const hittable& world, int num_cores = 1) {
        using namespace std::chrono;

        time_point start = high_resolution_clock::now();

        initialize();

        int scanned_lines = 0;
        auto render_region =
            [this, &start, &scanned_lines](
                const hittable& world, int start_y, int end_y,
                std::vector<std::vector<color>>& image_data) -> void {
            for (int j = start_y; j < end_y; ++j) {
                auto time = duration_cast<duration<double>>(
                    high_resolution_clock::now() - start);
                std::clog << std::format(
                    "Duration: {:.2f}, Scanlines remaining: {:03d}\r",
                    time.count(), (_image_height - ++scanned_lines));
                for (int i = 0; i < image_width; ++i) {
                    color pixel_color(0, 0, 0);
                    for (int sample = 0; sample < samples_per_pixel; ++sample) {
                        ray r = get_ray(i, j);
                        pixel_color += ray_color(r, max_depth, world);
                    }
                    image_data[j][i] = pixel_color;
                }
            }
        };

        std::vector<std::vector<color>> image_data(
            _image_height, std::vector<color>(image_width, color(0, 0, 0)));

        std::vector<std::thread> threads;

        int lines_per_core = _image_height / num_cores;
        for (int core = 0; core < num_cores; ++core) {
            int start_line = core * lines_per_core;
            int end_line = (core == num_cores - 1)
                               ? _image_height
                               : (core + 1) * lines_per_core;
            threads.emplace_back(render_region, std::ref(world), start_line,
                                 end_line, std::ref(image_data));
        }

        for (auto& thread : threads) {
            thread.join();
        }

        std::ofstream image("output.ppm");
        image << "P3\n" << image_width << ' ' << _image_height << "\n255\n";

        for (int j = 0; j < _image_height; ++j) {
            for (int i = 0; i < image_width; ++i) {
                write_color(image, image_data[j][i], samples_per_pixel);
            }
        }

        duration<double> time = duration_cast<duration<double>>(
            high_resolution_clock::now() - start);
        std::clog << std::format("\nDone in {:.2f} seconds.\n", time.count());
    }

private:
    int _image_height;     // Rendered image height
    point3 _center;        // Camera center
    point3 _pixel00_loc;   // Location of pixel 0, 0
    vec3 _pixel_delta_u;   // Offset to pixel to the right
    vec3 _pixel_delta_v;   // Offset to pixel below
    vec3 _u;               // Camera right
    vec3 _v;               // Camera up
    vec3 _w;               // Camera back
    vec3 _defocus_disk_u;  // Defocus disk horizontal radius
    vec3 _defocus_disk_v;  // Defocus disk vertical radius

    void initialize() {
        _image_height = static_cast<int>(image_width / aspect_ratio);
        _image_height = (_image_height < 1) ? 1 : _image_height;

        _center = lookfrom;

        // Determine viewport dimensions.
        auto theta = degrees_to_radians(vfov);
        auto h = tan(theta / 2);
        auto viewport_height = 2 * h * focus_dist;
        double viewport_width =
            viewport_height *
            (static_cast<double>(image_width) / _image_height);

        // Calculate the u,v,w unit basis vectors for the camera coordinate frame.
        _w = unit_vector(lookfrom - lookat);
        _u = unit_vector(cross(vup, _w));
        _v = cross(_w, _u);

        // Calculate the vectors across the horizontal and down the vertical viewport edges.
        vec3 viewport_u =
            viewport_width * _u;  // Vector across viewport horizontal edge
        vec3 viewport_v =
            viewport_height * -_v;  // Vector down viewport vertical edge

        // Calculate the horizontal and vertical delta vectors from pixel to pixel.
        _pixel_delta_u = viewport_u / image_width;
        _pixel_delta_v = viewport_v / _image_height;

        // Calculate the location of the upper left pixel.
        vec3 viewport_upper_left =
            _center - (focus_dist * _w) - viewport_u / 2 - viewport_v / 2;
        _pixel00_loc =
            viewport_upper_left + 0.5 * (_pixel_delta_u + _pixel_delta_v);

        // Calculate the camera defocus disk basis vectors.
        double defocus_radius =
            focus_dist * tan(degrees_to_radians(defocus_angle / 2));
        _defocus_disk_u = _u * defocus_radius;
        _defocus_disk_v = _v * defocus_radius;
    }

    ray get_ray(int i, int j) const {
        // Get a randomly-sampled camera ray for the pixel at location i,j, originating from
        // the camera defocus disk.

        vec3 pixel_center =
            _pixel00_loc + (i * _pixel_delta_u) + (j * _pixel_delta_v);
        vec3 pixel_sample = pixel_center + pixel_sample_square();

        point3 ray_origin =
            (defocus_angle <= 0) ? _center : defocus_disk_sample();
        vec3 ray_direction = pixel_sample - ray_origin;

        return ray(ray_origin, ray_direction);
    }

    vec3 pixel_sample_square() const {
        // Returns a random point in the square surrounding a pixel at the origin.
        double px = -0.5 + random_double();
        double py = -0.5 + random_double();
        return (px * _pixel_delta_u) + (py * _pixel_delta_v);
    }

    point3 defocus_disk_sample() const {
        // Returns a random point in the camera defocus disk.
        vec3 p = random_in_unit_disk();
        return _center + (p[0] * _defocus_disk_u) + (p[1] * _defocus_disk_v);
    }

    color ray_color(const ray& r, int depth, const hittable& world) const {
        if (depth <= 0) {
            return color(0, 0, 0);
        }

        hit_record rec;
        if (world.hit(r, interval(0.001, infinity), rec)) {
            ray scattered;
            color attenuation;
            if (rec.mat->scatter(r, rec, attenuation, scattered)) {
                return attenuation * ray_color(scattered, depth - 1, world);
            }

            return color(0, 0, 0);
        }

        // linear lerping
        vec3 unit_direction = unit_vector(r.direction());
        auto a = 0.5 * (unit_direction.y() + 1.0);
        return (1.0 - a) * color(1.0, 1.0, 1.0) + a * color(0.5f, 0.7f, 1.0f);
    }
};

}  // namespace rt
