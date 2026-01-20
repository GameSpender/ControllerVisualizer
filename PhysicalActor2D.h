#pragma once
#include "Actor.h"
#include "Physics.h"


class PhysicalActor2D : public Actor2D, public PhysicsReceiver{
public:


    // Physics component handles mass, forces, friction, torque, etc.
    std::shared_ptr<PhysicsComponent> physics;

    PhysicalActor2D() {
		physics = addComponent<PhysicsComponent>();
    }

    virtual ~PhysicalActor2D() = default;

    // Apply a force in world-space
    void applyForce(const glm::vec2& force, double dt) override {
        physics->applyForce(force, dt);
    }

    // Apply an instantaneous impulse
    void applyImpulse(const glm::vec2& impulse) override {
        physics->applyImpulse(impulse);
    }

    // Apply torque
    void applyTorque(float torque, double dt) override {
        physics->applyTorque(torque, dt);
    }

    void applyAngularImpulse(float impulse) override {
        physics->applyAngularImpulse(impulse);
    }

    void resetPhysics() override {
        physics->reset();
    }

    void update(double dt) override {
        physics->update(dt);
	}

    // Convenience getters
    glm::vec2 getVelocity() const { return physics->velocity; }
    float getAngularVelocity() const { return physics->angularVelocity; }
};