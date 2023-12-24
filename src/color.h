#pragma once

#include "pch.h"

namespace rt {

typedef vec3 color;

inline double linear_to_gamma(double linear_component) {
    return sqrt(linear_component);
}

inline void write_color(std::ostream& out, color pixel_color,
                        int samples_per_pixel) {
    double r = pixel_color.x();
    double g = pixel_color.y();
    double b = pixel_color.z();

    double scale = 1.0 / samples_per_pixel;
    r *= scale;
    g *= scale;
    b *= scale;

    r = linear_to_gamma(r);
    g = linear_to_gamma(g);
    b = linear_to_gamma(b);

    // Write the translated [0,255] value of each color component.
    static const interval intensity(0.000, 0.999);
    out << std::format("{} {} {}\n", static_cast<int>(256 * intensity.clamp(r)),
                       static_cast<int>(256 * intensity.clamp(g)),
                       static_cast<int>(256 * intensity.clamp(b)));
}

}  // namespace rt
