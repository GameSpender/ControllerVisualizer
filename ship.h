#pragma once
#include <cmath>
#include <glm/glm.hpp>
#include "SpriteRenderer.h"
#include "Transform2D.h"
#include "InteractionInterfaces.h"
#include "Projectile.h"
#include "irrKlang/irrKlang.h"

#include "PhysicalActor2D.h"
#include <string>
#include "Events.h"
#include "Weapon.h"
#include "Physics.h"
#include "HealthComponent.h"


#include "IControllable.h"
#include "Services.h"
#include "SoundManager.h"
#include "EventBus.h"



using namespace glm;


class Ship : public PhysicalActor2D, public IControllable
{

    std::shared_ptr<HealthComponent> health;

public:    
    // Ship parameters
    float baseThrust = 3000.0f;
    float bonusThrustMultiplier = 1.2f;
    float bonusBrakingMultiplier = 2.0f;
    float rotationThrust = 800.0f;

    float PD_p = 800.0f;
    float PD_d = 120.0f;

    float shotSpeed = 3000.0f;

    glm::vec2 thrustDir = glm::vec2(0.0f);   // local thrust 
    glm::vec2 targetRot = glm::vec2(0.0f);   // desired facing

    glm::vec2 screenMin = glm::vec2(0.0f);
    glm::vec2 screenMax = glm::vec2(0.0f);

    std::vector<std::shared_ptr<Hardpoint>> hardpoints;
    std::unordered_map<int, std::vector<std::shared_ptr<Hardpoint>>> actionMap;

    Ship() {
        physics->mass = 10.0f;
        physics->friction = 0.1f;
        physics->angularFriction = 1.0f;

        health = addComponent<HealthComponent>(100.0f, 10.0f, 0);

        // Subscribe to DeathEvent to mark ship as destroyed
        if (Services::eventBus) {
            Services::eventBus->process<DeathEvent>([this](const DeathEvent& e) {
                if (e.target == health.get()) {
                    for (auto hp : hardpoints)
                        hp->stopFiring();

                }
                });
        }

    }
    // -----------------------------------------
    //              MAIN UPDATE
    // -----------------------------------------
    void update(double dt) override
    {
        for (auto& hp : hardpoints) hp->update(dt);


        if (!isDead()) {
            glm::vec2 thrust = applyThrust(thrustDir, dt);
            float rotThrust = applyRotationThrust(targetRot, thrustDir, dt);
        }

		PhysicalActor2D::update(dt);

        if (screenMin != screenMax)
            borderCollision(screenMin, screenMax, 0.5f, -0.06f);

        markDirty();
    }

    void setThrust(const glm::vec2& input) {
        thrustDir = (glm::length(input) > 1.0f) ? glm::normalize(input) : input;
    }

    void setDirection(glm::vec2 dir) {
        targetRot = (glm::length(dir) > 0.05f) ? glm::normalize(dir) : glm::vec2(0.0f);
    }
	// IControllable implementation

    void setMoveDirection(const glm::vec2 dir) override {
		setThrust(dir);
    }

    void setAimDirection(const glm::vec2 dir) override {
		setDirection(dir);
    }

    void useAbility(int index, bool pressed) override {
        if (isDead()) return;
        if (index < 0 || index >= 4) return; // assuming abilities 1-4

        if (index < hardpoints.size()) {
            for (auto& [action, hpList] : actionMap) {
                for (auto& hp : hpList) {
                    if (pressed) hp->startFiring();
                    else hp->stopFiring();
                }
            }
        }
    }

    void dash(bool pressed) override {}

	// End of IControllable implementation



        // Forward health functions for convenience
    bool isDead() const { return health->isDead(); }
    void applyDamage(float amount, void* source = nullptr) { health->applyDamage(amount, source); }
    void heal(float amount) { health->heal(amount); }
    void respawnHealth() { health->respawn(); }


    void respawn(glm::vec2 pos, float rot = 0.0f) {
        position = pos;
        
        respawnHealth();
        resetPhysics();
        markDirty();
    }


    void addHardpoint(const std::shared_ptr<Hardpoint>& hp, int actionGroup) {
        hardpoints.push_back(hp);
        addChild(hp);
        actionMap[actionGroup].push_back(hp);
    }

private:

    // -----------------------------------------
    // Linear thrust (local → world)
    // -----------------------------------------
    glm::vec2 applyThrust(const glm::vec2& direction, float dt) {
        if (glm::length(direction) < 0.01f) return glm::vec2(0.0f);

        glm::vec2 thrustDirNorm = glm::normalize(direction);
        glm::vec2 forwardDir = forward();
        glm::vec2 thrustWorld = direction * baseThrust;

        float speed = glm::length(getVelocity());
        float totalMultiplier = 1.0f;

        float forwardAlignment = glm::dot(thrustDirNorm, forwardDir);
        if (forwardAlignment > 0.0f) totalMultiplier += forwardAlignment * bonusThrustMultiplier;

        if (speed > 0.001f) {
            glm::vec2 velDir = glm::normalize(getVelocity());
            float brakingAlignment = glm::dot(thrustDirNorm, -velDir);
            if (brakingAlignment > 0.0f) totalMultiplier += brakingAlignment * bonusBrakingMultiplier * glm::max(0.0f, forwardAlignment);
        }

        glm::vec2 finalThrust = thrustWorld * totalMultiplier;
        applyForce(finalThrust, dt);
        return finalThrust;
    }



    float applyRotationThrust(const glm::vec2& targetDir, const glm::vec2& thrustDir, float dt) {
        glm::vec2 dir = (glm::length(targetDir) > 0.001f) ? targetDir : thrustDir;
        if (glm::length(dir) < 0.001f) return 0.0f;

        float desiredAngle = glm::atan(dir.y, -dir.x) + glm::radians(90.0f);
        float angleDiff = desiredAngle - rotation;
        angleDiff = glm::mod(angleDiff + glm::pi<float>(), glm::two_pi<float>());
        if (angleDiff < 0.0f) angleDiff += glm::two_pi<float>();
        angleDiff -= glm::pi<float>();

        float torque = PD_p * angleDiff - PD_d * getAngularVelocity();
        torque = glm::clamp(torque, -rotationThrust, rotationThrust);

        applyTorque(torque, dt);
        return torque;
    }

    void borderCollision(const glm::vec2& screenMin, const glm::vec2& screenMax, float restitution = 0.5f, float spinFactor = 0.3f) {
		vec2 velocity = physics->velocity;
		float angularVelocity = physics->angularVelocity;

        // X-axis
        if (position.x < screenMin.x) { position.x = screenMin.x; 
            if (velocity.x < 0.0f) { velocity.x = -velocity.x * restitution; angularVelocity += velocity.y * spinFactor; } }
        else if (position.x > screenMax.x) { position.x = screenMax.x; 
            if (velocity.x > 0.0f) { velocity.x = -velocity.x * restitution; angularVelocity -= velocity.y * spinFactor; } }

        // Y-axis
        if (position.y < screenMin.y) { position.y = screenMin.y; 
            if (velocity.y < 0.0f) { velocity.y = -velocity.y * restitution; angularVelocity -= velocity.x * spinFactor; } }
        else if (position.y > screenMax.y) { position.y = screenMax.y; 
            if (velocity.y > 0.0f) { velocity.y = -velocity.y * restitution; angularVelocity += velocity.x * spinFactor; } }

		physics->velocity = velocity;
		physics->angularVelocity = angularVelocity;
    }
};


