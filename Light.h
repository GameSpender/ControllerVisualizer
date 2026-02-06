#pragma once
#include <glm/glm.hpp>
#include <memory>
#include "Transform2D.h"

class Light2D : public Transform2D {
public:
	glm::vec3 position; // offset from parent transform
    glm::vec3 color;

    // realistic falloff for 1 unit = 1 meter scale
    float intensity = 1.0f;
    float range = 100.0f;
    float falloff = 1.0f;


    vec3 getWorldPosition() const {
        if (auto p = parent.lock()) {
            return vec3(p->getWorldPosition(), 0) + position;
        }
        return position;
	}

	Light2D() : Transform2D() {}
    virtual ~Light2D() = default; // must be polymorphic for dynamic_cast
};

class PointLight2D : public Light2D {
};
