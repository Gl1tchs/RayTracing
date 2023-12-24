#pragma once

#include "vec3.h"

namespace rt {

class ray {
public:
    ray() {}

    ray(const vec3& origin, const vec3& direction)
        : _orig(origin), _dir(direction) {}

    point3 origin() const { return _orig; }
    vec3 direction() const { return _dir; }

    point3 at(double t) const { return _orig + t * _dir; }

private:
    point3 _orig;
    vec3 _dir;
};

};  // namespace rt
