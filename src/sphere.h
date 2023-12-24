#pragma once

#include "pch.h"

#include "hittable.h"

namespace rt {

class sphere : public hittable {
public:
    sphere(point3 center, double radius, std::shared_ptr<material> material)
        : _center(center), _radius(radius), _mat(material) {}

    bool hit(const ray& r, interval ray_t, hit_record& rec) const override {
        rt::vec3 oc = r.origin() - _center;
        double a = r.direction().length_squared();
        double half_b = rt::dot(oc, r.direction());
        double c = oc.length_squared() - _radius * _radius;
        double discriminant = half_b * half_b - a * c;
        double sqrtd = sqrt(discriminant);

        if (discriminant < 0) {
            return false;
        }

        // Find the nearest root that lies in the acceptable range.
        double root = (-half_b - sqrtd) / a;
        if (!ray_t.surrounds(root)) {
            root = (-half_b + sqrtd) / a;
            if (!ray_t.surrounds(root)) {
                return false;
            }
        }

        rec.t = root;
        rec.p = r.at(rec.t);
        vec3 outward_normal = (rec.p - _center) / _radius;
        rec.set_face_normals(r, outward_normal);
        rec.mat = _mat;

        return true;
    }

private:
    point3 _center;
    double _radius;
    std::shared_ptr<material> _mat;
};

}  // namespace rt
