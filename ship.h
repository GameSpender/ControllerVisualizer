#pragma once
#include <cmath>
#include <glm/glm.hpp>
#include "SpriteRenderer.h"
#include "Transform2D.h"
#include "InteractionInterfaces.h"
#include "Projectile.h"
#include "irrKlang/irrKlang.h"

#include "Actor.h"
#include <string>
#include "Events.h"
#include "Weapon.h"
#include "Physics.h"


#include "IControllable.h"
#include "Services.h"
#include "SoundManager.h"
#include "EventBus.h"



using namespace glm;


class Ship : public Actor2D, public IControllable
{

public:    
    PhysicsComponent physics;
    // inertia = (vx, vy, angularVelocity)
    float baseThrust = 3000.0f;
    float bonusThrustMultiplier = 1.2f;
    float bonusBrakingMultiplier = 2.0f;
    float rotationThrust = 800.0f;

    float PD_p = 800.0f;
    float PD_d = 120.0f;

    float shotSpeed = 3000.0f;

    glm::vec2 thrustDir = glm::vec2(0.0f);   // local thrust 

    glm::vec2 screenMin = glm::vec2(0.0f);
    glm::vec2 screenMax = glm::vec2(0.0f);


    glm::vec2 targetRot = glm::vec2(0.0f);

    std::vector<std::shared_ptr<Hardpoint>> hardpoints;

    std::unordered_map<int, std::vector<std::shared_ptr<Hardpoint>>> actionMap;

    bool destroyed = false;

    Ship() {
        physics.friction = 0.1f;
        physics.angularFriction = 1.0f;
        physics.mass = 10.0f;
    }
    // -----------------------------------------
    //              MAIN UPDATE
    // -----------------------------------------
    void update(double dt) override
    {
        float delta = (float)dt;

        position += velocity * delta;
        rotation += velocityRot * delta;

        

        
        for (auto hp : hardpoints) {
            hp->update(dt);
		}

        vec2 thrust = applyThrust(thrustDir, dt);
        float rotThrust = applyRotationThrust(targetRot, thrustDir, dt);
        
        float thrustIntensity = length(thrust) + rotThrust;

        physics.integrate(*this, dt);

        if(screenMin != screenMax)
            borderCollision(screenMin, screenMax, 0.5f, -0.06f);


        markDirty();            
    }

    void setThrust(const vec2& input)
    {
        if (length(input) > 1.0f)
        {
            thrustDir = normalize(input);
        }
        else
            thrustDir = input;
    }

    void setDirection(vec2 dir)
    {
        if (length(dir) > 0.05f)
            targetRot = normalize(dir);
        else
            targetRot = vec2(0.0f);
    }

	// IControllable implementation
    vec2 getPostion() override {
		return getWorldPosition();
    }

    void setMoveDirection(const glm::vec2 dir) override {
		setThrust(dir);
    }

    void setAimDirection(const glm::vec2 dir) override {
		setDirection(dir);
    }

    void useAbility(int index, bool pressed) override {
        if (index < 0 || index >= 4) return; // assuming abilities 1-4


        // Map index → hardpoints or other abilities
        if (index < hardpoints.size()) {
            for (auto& [action, hpList] : actionMap) {
                bool firing = pressed;
                for (auto& hp : hpList) {
                    if (firing) hp->startFiring();
                    else hp->stopFiring();
                }
            }
        }
    }

    void dash(bool pressed) override {
        if (pressed) {
            
            velocity += thrustDir * 2000.0f; // example dash impulse
        }
    }

	// End of IControllable implementation



    //vec2 getRelativeMouse() {
    //    if (hasInput()) {
    //        vec2 mousePos = input->getPosition(Action::MousePositionHorizontal, Action::MousePositionVertical);
    //        vec2 relative = mousePos - position;
    //        return relative;
    //    }
    //    return vec2(0);
    //}

    //void handleInput(double dt) {
    //    if (hasInput()) {
    //        setThrust(input->getPosition(Action::MoveHorizontal, Action::MoveVertical));

    //        vec2 aimDir = input->getPosition(Action::AimHorizontal, Action::AimVertical);
    //        if (input->isDown(Action::Aim)) {
    //            aimDir += getRelativeMouse();
    //        }
    //        setDirection(aimDir);

    //        bool click = input->isPressed(Action::Shoot);

    //        // Update all hardpoints based on the mapped input
    //        for (auto& [action, hpList] : actionMap) {
    //            bool firing = input->isDown(action);
    //            for (auto& hp : hpList) {
    //                if (firing) hp->startFiring();
    //                else hp->stopFiring();
    //                hp->update(dt);
    //            }
    //        }
    //    }
    //}


    bool checkHit(const Projectile& proj) {
        float dist = length(proj.position - position);
        return dist < length(scale) * 0.3f; // example radius
    }
    
    void setDestroyed()
    {
        if (!destroyed) {
            destroyed = true;
        }
        
    }

    void setRepaired()
    {
        if(destroyed)
            destroyed = false;
    }

    void respawn(glm::vec2 pos, float rot = 0.0f) {
        position = pos;
        
        setRepaired();
        markDirty();
    }


    void addHardpoint(const std::shared_ptr<Hardpoint>& hp, int fireAction) {
        hardpoints.push_back(hp);
        addChild(hp);

        actionMap[fireAction].push_back(hp);
    }


    void applyImpulse(const vec2& impulse) override {
        physics.applyImpulse(impulse);
    }
    void applyAngularImpulse(float angularImpulse) override {
        physics.applyAngularImpulse(angularImpulse);
    }

private:

    // -----------------------------------------
    // Linear thrust (local → world)
    // -----------------------------------------
    vec2 applyThrust(const vec2 direction, float delta)
    {
        if (length(direction) < 0.01f)
            return vec2(0.0f); // no input

        vec2 thrustDir = normalize(direction);        // normalized thrust input
        vec2 forwardDir = forward();                  // ship's facing
        vec2 thrustWorld = direction * baseThrust;    // base thrust in world space

        float speed = length(physics.velocity);

        float totalMultiplier = 1.0f;

        // ----- Forward bonus -----
        float forwardAlignment = dot(thrustDir, forwardDir); // -1..1
        if (forwardAlignment > 0.0f)
            totalMultiplier += forwardAlignment * bonusThrustMultiplier;

        // ----- Braking bonus -----
        if (speed > 0.001f)
        {
            vec2 velDir = normalize(physics.velocity);
            float brakingAlignment = dot(thrustDir, -velDir); // 1 = against velocity
            if (brakingAlignment > 0.0f)
                totalMultiplier += brakingAlignment * bonusBrakingMultiplier * glm::max(0.0f, forwardAlignment);
        }

        vec2 finalThrust = thrustWorld * totalMultiplier;


        // Apply final thrust
        physics.applyForce(finalThrust, delta);
        return finalThrust;
    }



    // -----------------------------------------
    // Rotational thrust
    // -----------------------------------------
    float applyRotationThrust(const glm::vec2& targetDir, const glm::vec2& thrustDir, float delta)
    {
        // ----------------------
        // 1. Determine desired rotation
        // ----------------------
        glm::vec2 dir = glm::length(targetDir) > 0.001f ? targetDir : thrustDir;

        if (glm::length(dir) < 0.001f)
            return 0.0f; // no input, do nothing

        float desiredAngle = glm::atan(dir.y, -dir.x) + glm::radians(90.0f);

        // ----------------------
        // 2. Compute shortest angular difference
        // ----------------------
        float angleDiff = desiredAngle - rotation;
        angleDiff = glm::mod(angleDiff + glm::pi<float>(), glm::two_pi<float>());
        if (angleDiff < 0.0f) angleDiff += glm::two_pi<float>();
        angleDiff -= glm::pi<float>();

        // ----------------------
        // 3. PD Controller
        // ----------------------

        float torque = PD_p * angleDiff - PD_d * physics.angularVelocity; // P-D control

        // Optional clamp to prevent extreme angular acceleration
        torque = glm::clamp(torque, -rotationThrust, rotationThrust);

        // ----------------------
        // 4. Apply torque
        // ----------------------
        physics.applyTorque(torque, delta);
        return torque;
    }

    // Call inside Ship::update() AFTER physics.integrate()
    void borderCollision(const glm::vec2& screenMin,
        const glm::vec2& screenMax,
        float restitution = 0.5f,
        float spinFactor = 0.3f)
    {
        // X-axis collision
        if (position.x < screenMin.x)
        {
            position.x = screenMin.x;

            if (physics.velocity.x < 0.0f)
            {
                physics.velocity.x = -physics.velocity.x * restitution;
                physics.angularVelocity += physics.velocity.y * spinFactor;
            }
        }
        else if (position.x > screenMax.x)
        {
            position.x = screenMax.x;

            if (physics.velocity.x > 0.0f)
            {
                physics.velocity.x = -physics.velocity.x * restitution;
                physics.angularVelocity -= physics.velocity.y * spinFactor;
            }
        }

        // Y-axis collision
        if (position.y < screenMin.y)
        {
            position.y = screenMin.y;

            if (physics.velocity.y < 0.0f)
            {
                physics.velocity.y = -physics.velocity.y * restitution;
                physics.angularVelocity -= physics.velocity.x * spinFactor;
            }
        }
        else if (position.y > screenMax.y)
        {
            position.y = screenMax.y;

            if (physics.velocity.y > 0.0f)
            {
                physics.velocity.y = -physics.velocity.y * restitution;
                physics.angularVelocity += physics.velocity.x * spinFactor;
            }
        }
    }


    void produceParticles(float thrust, double dt) {
        if (thrust < 0.001f)
            return;

        // Number of particles scales with thrust and delta time
        float thrustSmoke = thrust * 70.0f * dt;
        int particlesToSpawn = 0;

        if (thrustSmoke >= 1.0f) {
            particlesToSpawn = static_cast<int>(thrustSmoke);
        }
        else {
            // Chance-based: spawn 1 particle with probability equal to thrustSmoke
            if ((float(rand()) / RAND_MAX) < thrustSmoke) {
                particlesToSpawn = 1;
            }
        }

        for (int i = 0; i < particlesToSpawn; ++i) {
            // Spawn position slightly behind the ship
            vec2 offset = -forward() * 0.3f * length(getWorldScale()); // 10 units behind ship
            vec2 spawnPos = position + offset;

            // Particle velocity opposite to ship's forward + some randomness
            vec2 particleVel = -forward() * (10.0f + thrust * 100.0f) + physics.velocity;
            particleVel += vec2(
                (float(rand()) / RAND_MAX - 0.5f) * 10.0f, // x random jitter
                (float(rand()) / RAND_MAX - 0.5f) * 10.0f  // y random jitter
            );

            // Lifetime, size, color (customize as needed)
            float lifetime = 0.2f + (float(rand()) / RAND_MAX) * 1.0f;
            float size = 10.0f + (float(rand()) / RAND_MAX) * 2.0f;
            float rot = (float(rand()) / RAND_MAX) * M_PI;
            vec4 color = vec4(0.8f, 0.8f, 0.8f, 1.0f); // light gray smoke

            // Add to the vector
            //smokeParticles.emplace_back(smokeTexture, spawnPos, particleVel, lifetime, size, rot, 3.0f);
        }
    }
};


