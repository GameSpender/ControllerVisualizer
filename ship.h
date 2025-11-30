#pragma once
#include <cmath>
#include <glm/glm.hpp>
#include "SpriteRenderer.h"
#include "Transform2D.h"
#include "InteractionInterfaces.h"
#include "Projectile.h"
#include "irrKlang/irrKlang.h"


using namespace glm;


class Ship : public Transform2D, public Animated
{
    
    unsigned int texture;
    unsigned int projectileTexture;
    
    unsigned int smokeTexture;
    vector<Projectile> smokeParticles;

    double nextParticle = 0.0f;
    double particleInterval = 0.05f;

    double nextShot = 0.0f;
    double shotInterval = 0.1f;
public:
    vector<Projectile> projectiles;
    
    function<void()> shootCallback;
    function<void()> destroyedCallback;
    function<void(float)> thrustCallback;

    // inertia = (vx, vy, angularVelocity)
    vec2 inertiaLinear = vec2(0.0f);
    float inertiaAngular = 0.0f;
    float baseThrust = 400.0f;
    float bonusThrustMultiplier = 1.2f;
    float bonusBrakingMultiplier = 2.0f;
    float rotationThrust = 100.0f;

    float PD_p = 45.0f;
    float PD_d = 9.0f;

    float shotSpeed = 3000.0f;

    vec2 thrustDir = vec2(0.0f);   // local thrust 

    float mass = 5.0f;
    float friction = 0.1f;
    float rotationFriction = 1.0f;

    vec2 screenMin = vec2(0.0f);
    vec2 screenMax = vec2(0.0f);


    vec2 targetRot = vec2(0.0f);

    bool destroyed = false;

    Ship(unsigned int texture, unsigned int projectileTexture, unsigned int smokeTexture) 
        : texture(texture), projectileTexture(projectileTexture), smokeTexture(smokeTexture){}
    Ship(unsigned int texture, unsigned int projectileTexture, unsigned int smokeTexture, int screenWidth, int screenHeight)
        : texture(texture), projectileTexture(projectileTexture), smokeTexture(smokeTexture),
        screenMin(vec2(0.0f)), screenMax(vec2(screenWidth, screenHeight)) {}

    // -----------------------------------------
    //              MAIN UPDATE
    // -----------------------------------------
    void update(double dt) override
    {
        float delta = (float)dt;

        position += inertiaLinear * delta;
        rotation += inertiaAngular * delta;

        vec2 thrust;
        // Apply forces
        if (!destroyed) {
            thrust = applyThrust(thrustDir, delta);
            applyRotationThrust(targetRot, thrustDir, delta);

            produceParticles(length(thrust) / baseThrust, dt);
        }
        

        //produceParticles(1.0f, dt);
        // Movement integration
        inertiaLinear -= inertiaLinear * friction * delta;

        // Rotation integration
        inertiaAngular -= inertiaAngular * rotationFriction * delta;

        borderCollision(screenMin, screenMax, 0.5f, -0.06f);

        updateParticles(dt);
        updateProjectiles(dt);

        markDirty();
    }

    void Draw(SpriteRenderer& renderer) override
    {
        renderer.Draw(texture, getWorldMatrix());

        for (auto p : projectiles)
            p.Draw(renderer);
        for (auto p : smokeParticles)
            p.Draw(renderer);
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

    vec2 forward() {
        float rot = -rotation - radians(90.0f);
        return vec2(cos(rot), sin(rot));
    }

    void shoot() { 
        if (destroyed) return;
        double currentTime = glfwGetTime();
        if (currentTime > nextShot) {
            vec2 spawnPos = position + forward() * 20.0f; // spawn in front of ship
            vec2 projVelocity = forward() * shotSpeed + inertiaLinear;       // projectile speed
            projectiles.emplace_back(projectileTexture, spawnPos, projVelocity, 3.0f, 25.0f, rotation);
            projectiles.back().setParent(parent);
            nextShot = currentTime + shotInterval;

            shootCallback();
        }
    }

    bool checkHit(const Projectile& proj) {
        float dist = length(proj.position - position);
        return dist < length(scale) * 0.3f; // example radius
    }
    
    void setDestroyed()
    {
        if (!destroyed) {
            destroyed = true;
            destroyedCallback();
        }
        
    }

    void setRepaired()
    {
        if(destroyed)
            destroyed = false;
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

        float speed = length(inertiaLinear);

        float totalMultiplier = 1.0f;

        // ----- Forward bonus -----
        float forwardAlignment = dot(thrustDir, forwardDir); // -1..1
        if (forwardAlignment > 0.0f)
            totalMultiplier += forwardAlignment * bonusThrustMultiplier;

        // ----- Braking bonus -----
        if (speed > 0.001f)
        {
            vec2 velDir = normalize(inertiaLinear);
            float brakingAlignment = dot(thrustDir, -velDir); // 1 = against velocity
            if (brakingAlignment > 0.0f)
                totalMultiplier += brakingAlignment * bonusBrakingMultiplier * glm::max(0.0f, forwardAlignment);
        }

        vec2 finalThrust = thrustWorld * totalMultiplier;
        if(thrustCallback)
            thrustCallback(length(finalThrust));
        // Apply final thrust
        inertiaLinear += finalThrust * delta;
        return finalThrust;
    }



    // -----------------------------------------
    // Rotational thrust
    // -----------------------------------------
    void applyRotationThrust(const glm::vec2& targetDir, const glm::vec2& thrustDir, float delta)
    {
        // ----------------------
        // 1. Determine desired rotation
        // ----------------------
        glm::vec2 dir = glm::length(targetDir) > 0.001f ? targetDir : thrustDir;

        if (glm::length(dir) < 0.001f)
            return; // no input, do nothing

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

        float torque = PD_p * angleDiff - PD_d * inertiaAngular; // P-D control

        // Optional clamp to prevent extreme angular acceleration
        torque = glm::clamp(torque, -rotationThrust, rotationThrust);

        // ----------------------
        // 4. Apply torque
        // ----------------------
        inertiaAngular += torque * delta;
    }

    // Call this inside update() after moving the ship
    void borderCollision(const glm::vec2& screenMin, const glm::vec2& screenMax, float restitution = 0.5f, float spinFactor = 0.3f)
    {
        // Check X axis
        if (position.x < screenMin.x)
        {
            position.x = screenMin.x; // clamp to border
            if (inertiaLinear.x < 0)
            {
                inertiaLinear.x = -inertiaLinear.x * restitution; // bounce
                inertiaAngular += inertiaLinear.y * spinFactor;   // spin based on perpendicular velocity
            }
        }
        else if (position.x > screenMax.x)
        {
            position.x = screenMax.x;
            if (inertiaLinear.x > 0)
            {
                inertiaLinear.x = -inertiaLinear.x * restitution;
                inertiaAngular -= inertiaLinear.y * spinFactor;
            }
        }

        // Check Y axis
        if (position.y < screenMin.y)
        {
            position.y = screenMin.y;
            if (inertiaLinear.y < 0)
            {
                inertiaLinear.y = -inertiaLinear.y * restitution;
                inertiaAngular -= inertiaLinear.x * spinFactor;
            }
        }
        else if (position.y > screenMax.y)
        {
            position.y = screenMax.y;
            if (inertiaLinear.y > 0)
            {
                inertiaLinear.y = -inertiaLinear.y * restitution;
                inertiaAngular += inertiaLinear.x * spinFactor;
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
            vec2 particleVel = -forward() * (10.0f + thrust * 100.0f) + inertiaLinear;
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
            smokeParticles.emplace_back(smokeTexture, spawnPos, particleVel, lifetime, size, rot, 3.0f);
        }
    }

    void updateProjectiles(double dt) {
        for (int i = projectiles.size() - 1; i >= 0; --i) {
            projectiles[i].update(dt);

            if (projectiles[i].isDead()) {
                projectiles.erase(projectiles.begin() + i); // safe, no memory leaks
            }
        }
    }

    void updateParticles(double dt) {
        for (int i = smokeParticles.size() - 1; i >= 0; --i) {
            smokeParticles[i].update(dt);

            if (smokeParticles[i].isDead()) {
                smokeParticles.erase(smokeParticles.begin() + i); // safe, no memory leaks
            }
        }
    }
};