#pragma once
#include <glm/glm.hpp>
#include <memory>
#include "Transform3D.h"

class Light3D : public Transform3D {
public:
    glm::vec3 color;

    // realistic falloff for 1 unit = 1 meter scale
    float intensity = 1.0f;
    float range = 100.0f;
    float falloff = 1.0f;


	Light3D() : Transform3D() {}
    virtual ~Light3D() = default; // must be polymorphic for dynamic_cast
};

class PointLight3D : public Light3D {
};
