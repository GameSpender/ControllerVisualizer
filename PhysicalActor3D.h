#pragma once
#include "Actor3D.h"
#include "Physics3D.h"

class PhysicalActor3D : public Actor3D, public PhysicsReceiver3D {
public:
    // Physics component handles mass, forces, friction, torque, etc.
    std::shared_ptr<PhysicsComponent3D> physics;

    PhysicalActor3D() {
		name = "PhysicalActor3D";
        physics = addComponent<PhysicsComponent3D>();
    }

    virtual ~PhysicalActor3D() = default;

    // ---------------- PhysicsReceiver3D interface ----------------

    void applyForce(const glm::vec3& force, double dt) override {
        physics->applyForce(force, dt);
    }

    void applyImpulse(const glm::vec3& impulse) override {
        physics->applyImpulse(impulse);
    }

    void applyTorque(const glm::vec3& torque, double dt) override {
        physics->applyTorque(torque, dt);
    }

    void applyAngularImpulse(const glm::vec3& impulse) override {
        physics->applyAngularImpulse(impulse);
    }

    void resetPhysics() override {
        physics->reset();
    }

    // ---------------- Actor update ----------------
    void update(double dt) override {
        Actor3D::update(dt);   // update all components (including physics)
        markDirty();
    }

    // ---------------- Convenience getters ----------------
    glm::vec3 getVelocity() const { return physics->velocity; }
    glm::vec3 getAngularVelocity() const { return physics->angularVelocity; }
};
