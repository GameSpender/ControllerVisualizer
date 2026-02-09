#pragma once
#include "glm/glm.hpp"
#include "glm/gtc/quaternion.hpp"
#include "BaseComponent3D.h"
#include "Actor3D.h"

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
    float mass = 1.0f;
    float friction = 0.2f;           // linear damping
    float angularFriction = 0.05f;    // rotational damping

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
            glm::vec3 axis = glm::normalize(angularVelocity);
            float angle = glm::length(angularVelocity) * float(dt);
            glm::quat rotDelta = glm::angleAxis(angle, axis);
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

namespace Physics {
    glm::vec3 convertVelocityToNewWorld(const glm::vec3& worldVel,
        const glm::mat4& oldWorld,
        const glm::mat4& newWorld)
    {

        glm::vec3 localVel = glm::inverse(oldWorld) * glm::vec4(worldVel, 0.0f); // 0 = vector
        auto vel =  glm::vec3(newWorld * glm::vec4(localVel, 0.0f));
		vel.z = -vel.z; // Invert Z to convert between left-handed and right-handed
        return vel;
    }
}
