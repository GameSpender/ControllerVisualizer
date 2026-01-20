#pragma once
#include "Actor.h"
#include "ProjectileSystem.h"
#include "Events.h"
#include <random>
#include "Physics.h"

#include "Services.h"
#include "EventBus.h"
#include "ProjectileSystem.h"


class Weapon : public Actor2D{
protected:
	
public:
	double shotInterval;
	double nextShot;

    float damage;
    int team;
    float deviation;
    float recoil;


    virtual void startFiring() {};
    virtual void stopFiring() {};

};


class Hardpoint : public Transform2D, public PhysicsReceiver {
public:
    std::shared_ptr<Weapon> weapon;

private:
    PhysicsReceiver* cachedParentReceiver = nullptr;

    PhysicsReceiver* getParentReceiver() {
        if (cachedParentReceiver)
            return cachedParentReceiver;

        if (auto par = parent.lock()) {
            if (auto receiver = dynamic_cast<PhysicsReceiver*>(par.get())) {
                cachedParentReceiver = receiver;
                return receiver;
            }
        }

        return nullptr;
    }


public:

    Hardpoint() = default;

    void attachWeapon(const std::shared_ptr<Weapon>& w) {
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

    // Pass firing command to attached weapon if it exists
    void startFiring() {
        if (weapon)
            weapon->startFiring();
    }

    // Pass firing command to attached weapon if it exists
    void stopFiring() {
        if (weapon)
            weapon->stopFiring();
    }

    // Pass update to weapon if it exists
    void update(double dt) override {
        if (weapon)
            weapon->update(dt);
    }

    // Apply force to physically enabled parent
    void applyForce(const glm::vec2& force, double dt) override {
        if (auto receiver = getParentReceiver())
            receiver->applyForce(force, dt);
    }

    // Apply torque to physically enabled parent
    void applyTorque(float torque, double dt) override {
        if (auto receiver = getParentReceiver())
            receiver->applyTorque(torque, dt);
    }

    // Apply impulse to physically enabled parent
    void applyImpulse(const glm::vec2& impulse) override {
        if (auto receiver = getParentReceiver())
            receiver->applyImpulse(impulse);
    }

    // Apply angular impulse to physically enabled parent
    void applyAngularImpulse(float angularImpulse) override {
        if (auto receiver = getParentReceiver())
            receiver->applyAngularImpulse(angularImpulse);
    }
};

