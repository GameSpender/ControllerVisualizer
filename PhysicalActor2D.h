#pragma once
#include "Actor.h"
#include "Physics.h"


class PhysicalActor2D : public Actor2D, public PhysicsReceiver{
public:
    // Kinematics
    glm::vec2 velocity{ 0.0f };
    float angularVelocity{ 0.0f };

    bool isKinematic = false; // true = ignore forces/torque

    // Physics component handles mass, forces, friction, torque, etc.
    PhysicsComponent physics;

    PhysicalActor2D() {
        // Default physical parameters
        physics.mass = 1.0f;
        physics.friction = 0.1f;
        physics.angularFriction = 1.0f;
    }

    virtual ~PhysicalActor2D() = default;

    // Apply a force in world-space
    void applyForce(const glm::vec2& force, double dt) override {
        if (isKinematic) return;
        physics.applyForce(*this, force, dt);
    }

    // Apply an instantaneous impulse
    void applyImpulse(const glm::vec2& impulse) override {
        if (isKinematic) return;
        physics.applyImpulse(*this, impulse);
    }

    // Apply torque
    void applyTorque(float torque, double dt) override {
        if (isKinematic) return;
        physics.applyTorque(*this, torque, dt);
    }

    void applyAngularImpulse(float impulse) override {
        if (isKinematic) return;
        physics.applyAngularImpulse(*this, impulse);
    }

    void resetPhysics() override {
        velocity = glm::vec2(0.0f);
        angularVelocity = 0.0f;
        physics.reset(*this);
    }

    // Integrate kinematics based on velocity, angularVelocity, and physics
    virtual void integrate(double dt) {
        physics.integrate(*this, dt, isKinematic);
    }

    // Override update for derived classes
    virtual void update(double dt) override {
        integrate(dt);
    }

    // Convenience getters
    glm::vec2 getVelocity() const { return velocity; }
    float getAngularVelocity() const { return angularVelocity; }
};