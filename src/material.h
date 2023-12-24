#pragma once

#include "pch.h"

#include "color.h"
#include "hittable.h"
#include "vec3.h"

namespace rt {

class material {
public:
    virtual ~material() = default;

    virtual bool scatter(const ray& r_in, const hit_record& rec,
                         color& attenuation, ray& scattered) const = 0;
};

class lambertian : public material {
public:
    lambertian(const color& a) : _albedo(a) {}

    bool scatter(const ray& r_in, const hit_record& rec, color& attenuation,
                 ray& scattered) const override {
        auto scatter_direction = rec.normal + random_unit_vector();

        // Catch degenerate scatter direction
        if (scatter_direction.near_zero()) {
            scatter_direction = rec.normal;
        }

        scattered = ray(rec.p, scatter_direction);
        attenuation = _albedo;
        return true;
    }

private:
    color _albedo;
};

class metal : public material {
public:
    metal(const color& a, double f) : _albedo(a), _fuzz(f) {}

    bool scatter(const ray& r_in, const hit_record& rec, color& attenuation,
                 ray& scattered) const override {
        vec3 reflected = reflect(unit_vector(r_in.direction()), rec.normal);
        scattered = ray(rec.p, reflected + _fuzz * random_unit_vector());
        attenuation = _albedo;
        return dot(scattered.direction(), rec.normal) > 0.0;
    }

private:
    color _albedo;
    double _fuzz;
};

class dielectric : public material {
public:
    dielectric(double index_of_refraction) : _ir(index_of_refraction) {}

    bool scatter(const ray& r_in, const hit_record& rec, color& attenuation,
                 ray& scattered) const override {
        attenuation = color(1.0, 1.0, 1.0);
        // snell's law: sin(θ′)=(η / η′) * sin(θ)
        double refraction_ratio = rec.front_face ? (1.0 / _ir) : _ir;

        vec3 unit_direction = unit_vector(r_in.direction());
        // cos(θ) = dot(R, n)
        double cos_theta = fmin(dot(-unit_direction, rec.normal), 1.0);
        // sin(θ) = sqrt(1 − cos(θ)^2)
        double sin_theta = sqrt(1.0 - cos_theta * cos_theta);

        // sinθ cannot be larger than 1.0
        bool cannot_refract = refraction_ratio * sin_theta > 1.0;

        vec3 direction;
        if (cannot_refract ||
            reflectance(cos_theta, refraction_ratio) > random_double()) {
            direction = reflect(unit_direction, rec.normal);
        } else {
            direction = refract(unit_direction, rec.normal, refraction_ratio);
        }

        scattered = ray(rec.p, direction);
        return true;
    }

private:
    double _ir;  // Index of Refraction

    static double reflectance(double cosine, double ref_idx) {
        // Use Schlick's approximation for reflectance.
        double r0 = (1 - ref_idx) / (1 + ref_idx);
        r0 = r0 * r0;
        return r0 + (1 - r0) * pow((1 - cosine), 5);
    }
};

}  // namespace rt
