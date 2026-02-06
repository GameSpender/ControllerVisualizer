#pragma once
#include "glm/glm.hpp"
#include "glm/gtc/quaternion.hpp"
#include "BaseComponent3D.h"

class PhysicalActor3D;

// Interface for objects that can receive physical forces/torques
class PhysicsReceiver3D {
public:
    virtual ~PhysicsReceiver3D() = default;

    // Apply a continuous force in world-space
    virtual void applyForce(const glm::vec3& force, double dt) = 0;

    // Apply an instantaneous impulse
    virtual void applyImpulse(const glm::vec3& impulse) = 0;

    // Apply torque (rotational force) in world-space
    virtual void applyTorque(const glm::vec3& torque, double dt) = 0;

    // Apply an instantaneous angular impulse
    virtual void applyAngularImpulse(const glm::vec3& impulse) = 0;

    // Optional: reset kinematic state
    virtual void resetPhysics() {}
};

// --- Physics data / integrator ---
class PhysicsComponent3D : public BaseComponent3D, public PhysicsReceiver3D {
public:
    float mass = 5.0f;
    float friction = 0.5f;           // linear damping
    float angularFriction = 0.5f;    // rotational damping

    bool isKinematic = false;        // true = ignore forces/torque

    glm::vec3 velocity{ 0.0f };      // linear velocity
    glm::vec3 angularVelocity{ 0.0f }; // rotational velocity (Euler)

    PhysicsComponent3D() = default;

    // Integrate physics for this timestep
    void update(double dt) override {
        if (isKinematic) return;

        // Linear motion
        velocity *= std::max(0.0f, 1.0f - friction * float(dt));
        owner->position += velocity * float(dt);

        // Rotational motion
        angularVelocity *= std::max(0.0f, 1.0f - angularFriction * float(dt));
        if (glm::length(angularVelocity) > 0.0f) {
            glm::quat rotDelta = glm::quat(angularVelocity * float(dt)); // small-angle approximation
            owner->rotation = glm::normalize(rotDelta * owner->rotation);
        }
    }

    void applyForce(const glm::vec3& force, double dt) override {
        if (isKinematic || mass <= 0.0f) return;
        glm::vec3 acceleration = force / mass;
        velocity += acceleration * float(dt);
    }

    void applyImpulse(const glm::vec3& impulse) override {
        if (isKinematic || mass <= 0.0f) return;
        velocity += impulse / mass;
    }

    void applyTorque(const glm::vec3& torque, double dt) override {
        if (isKinematic) return;
        angularVelocity += torque * float(dt); // For now assume unit inertia
    }

    void applyAngularImpulse(const glm::vec3& impulse) override {
        if (isKinematic) return;
        angularVelocity += impulse; // For now assume unit inertia
    }

    void reset() {
        velocity = glm::vec3(0.0f);
        angularVelocity = glm::vec3(0.0f);
    }
};
