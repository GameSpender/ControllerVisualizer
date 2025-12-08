#pragma once
#include "glm/glm.hpp"
#include "Transform2D.h"

class PhysicsComponent {
public:
    glm::vec2 velocity{};
    float angularVelocity = 0.0f;
    float mass = 5.f;
    float friction = 0.0f;
    float angularFriction = 0.0f;

    void integrate(Transform2D& t, float dt)
    {
        // --- 1. Integrate position and rotation ---
        t.position += velocity * dt;
        t.rotation += angularVelocity * dt;

        if (friction > 0.0f)
            velocity -= velocity * friction * dt;

        if (angularFriction > 0.0f)
            angularVelocity -= angularVelocity * angularFriction * dt;
    }
    void applyForce(const glm::vec2& f, float dt) {
        velocity += f * dt / mass;
    }
    void applyTorque(float t, float dt) {
        angularVelocity += t * dt / mass;
    }

    void applyImpulse(const vec2& I) {
        velocity += I / mass;
    }

    void applyAngularImpulse(float J) {
        angularVelocity += J / mass;
    }

    void reset() {
        velocity = vec2(0);
        angularVelocity = 0.0f;
    }
};

class PhysicsReceiver {
public:
    virtual void applyForce(const glm::vec2& force) {}
    virtual void applyTorque(float torque) {}
    virtual void applyImpulse(const vec2& impulse) {}
    virtual void applyAngularImpulse(float angularImpulse) {}
};

