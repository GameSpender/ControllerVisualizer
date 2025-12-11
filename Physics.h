#pragma once
#include "glm/glm.hpp"


class PhysicalActor2D;

// Interface for objects that can receive physical forces/torques
class PhysicsReceiver {
public:
    virtual ~PhysicsReceiver() = default;

    // Apply a continuous force in world-space
    virtual void applyForce(const glm::vec2& force, double dt) = 0;

    // Apply an instantaneous impulse
    virtual void applyImpulse(const glm::vec2& impulse) = 0;

    // Apply torque (rotational force)
    virtual void applyTorque(float torque, double dt) = 0;

    // Apply an instantaneous angular impulse
    virtual void applyAngularImpulse(float impulse) = 0;

    // Optional: reset kinematic state
    virtual void resetPhysics() {}
};



// --- Physics data / integrator ---
class PhysicsComponent {
public:
    float mass = 5.0f;
    float friction = 0.0f;
    float angularFriction = 0.0f;

    PhysicsComponent() = default;

    void integrate(PhysicalActor2D& actor, double dt, bool isKinematic = false);
    void applyForce(PhysicalActor2D& actor, const glm::vec2& force, double dt);
    void applyTorque(PhysicalActor2D& actor, float torque, double dt);
    void applyImpulse(PhysicalActor2D& actor, const glm::vec2& impulse);
    void applyAngularImpulse(PhysicalActor2D& actor, float impulse);
    void reset(PhysicalActor2D& actor);
};
