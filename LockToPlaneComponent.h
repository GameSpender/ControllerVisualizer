#pragma once
#include "BaseComponent3D.h"
#include "Actor3D.h"
#include "Physics3D.h"
#include "glm/glm.hpp"

class LockToPlaneComponent : public BaseComponent3D {
public:
    glm::vec3 upVector = glm::vec3(0, 0, 1); // plane normal
    float offset = 0.0f;                     // distance along normal
    bool autoCaptureOffset = true;

private:
    bool initialized = false;

public:
    LockToPlaneComponent(const glm::vec3& up, bool autoCapture = true)
		: upVector(up), autoCaptureOffset(autoCapture) {
	}

    void update(double dt) override {
        if (!owner) return;

        glm::vec3 n = glm::normalize(upVector);

        // Capture offset from first frame if desired
        if (autoCaptureOffset && !initialized) {
            offset = glm::dot(owner->position, n);
            initialized = true;
        }

        // --- Project position onto plane ---
        float current = glm::dot(owner->position, n);
        float distance = current - offset;

        owner->position -= n * distance;

        // --- Remove velocity component along plane normal ---
        if (auto phys = owner->getComponent<PhysicsComponent3D>()) {
            float vAlongNormal = glm::dot(phys->velocity, n);
            phys->velocity -= n * vAlongNormal;
        }
    }
};
