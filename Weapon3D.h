#pragma once
#include "Actor3D.h"
#include "ProjectileSystem.h"
#include "Events.h"
#include <random>
#include "Physics3D.h"

#include "Services.h"
#include "EventBus.h"


// ---------------- Weapon Base ----------------
class Weapon3D : public Actor3D {
public:
    double shotInterval = 0.1;
    double nextShot = 0.0;

    float damage = 10.0f;
    int team = 0;
    float deviation = 0.0f;
    float recoil = 0.0f;

    virtual void startFiring() {};
    virtual void stopFiring() {};
};

// ---------------- Hardpoint ----------------
class Hardpoint3D : public Transform3D, public PhysicsReceiver3D {
public:
    std::shared_ptr<Weapon3D> weapon;

private:
    PhysicsReceiver3D* cachedParentReceiver = nullptr;

    PhysicsReceiver3D* getParentReceiver() {
        if (cachedParentReceiver)
            return cachedParentReceiver;

        if (auto par = parent.lock()) {
            if (auto receiver = dynamic_cast<PhysicsReceiver3D*>(par.get())) {
                cachedParentReceiver = receiver;
                return receiver;
            }
        }
        return nullptr;
    }

public:
    Hardpoint3D() = default;

    void attachWeapon(const std::shared_ptr<Weapon3D>& w) {
        weapon = w;
        if (weapon)
            addChild(weapon); // weapon follows hardpoint
    }

    void detachWeapon() {
        if (weapon) {
            removeChild(weapon);
            weapon->stopFiring();
            weapon.reset();
        }
    }

    void startFiring() {
        if (weapon)
            weapon->startFiring();
    }

    void stopFiring() {
        if (weapon)
            weapon->stopFiring();
    }

    void update(double dt) override {
        if (weapon)
            weapon->update(dt);
    }

    // ---------------- Physics forwarding ----------------
    void applyForce(const glm::vec3& force, double dt) override {
        if (auto receiver = getParentReceiver())
            receiver->applyForce(force, dt);
    }

    void applyTorque(const glm::vec3& torque, double dt) override {
        if (auto receiver = getParentReceiver())
            receiver->applyTorque(torque, dt);
    }

    void applyImpulse(const glm::vec3& impulse) override {
        if (auto receiver = getParentReceiver())
            receiver->applyImpulse(impulse);
    }

    void applyAngularImpulse(const glm::vec3& angularImpulse) override {
        if (auto receiver = getParentReceiver())
            receiver->applyAngularImpulse(angularImpulse);
    }
};
