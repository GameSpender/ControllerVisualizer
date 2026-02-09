#pragma once
#include <glm/glm.hpp>
#include "PhysicalActor3D.h"
#include "HealthComponent.h"
#include "IControllable.h"
#include "LockToPlaneComponent.h"
#include "Services.h"
#include "EventBus.h"
#include "UpdateSystem.h"
#include <string>
#include <unordered_map>
#include <vector>
#include "Weapon3D.h"
#include "ParticleEmitter.h"
#include "glm/gtx/vector_angle.hpp"
#include "Model3D.h"


using namespace glm;

class Ship3D : public PhysicalActor3D, public IControllable {
    std::shared_ptr<HealthComponent> health;
    std::shared_ptr<LockToPlaneComponent> planeLock;
    std::weak_ptr<Model3D> body;

public:

    std::weak_ptr<ParticleEmitter3D> engineEmitter;
    // Ship parameters
    float baseThrust = 1000.0f;
    float bonusThrustMultiplier = 1.4f;
    float bonusBrakingMultiplier = 2.0f;
    float rotationThrust = 170.0f;

    float roll = 0.0f;
    float rollSpeed = 0.0f;
    float centering = 20.0f;
    float spinFactor = 1.0f;
    float damping = 5.0f;

    float PD_p = 40.0f;
    float PD_d = 10.0f;

    float shotSpeed = 3000.0f;


    vec2 thrustDir = vec2(0.0f);   // local planar thrust
    vec2 targetRot = vec2(0.0f);   // desired facing in plane

    vec2 screenMin = vec2(0.0f);
    vec2 screenMax = vec2(0.0f);

    std::vector<std::shared_ptr<Hardpoint3D>> hardpoints;
    std::unordered_map<int, std::vector<std::shared_ptr<Hardpoint3D>>> actionMap;

    Ship3D() {
        physics->mass = 10.0f;
        physics->friction = 0.1f;
        physics->angularFriction = 1.0f;

        // Lock movement to Y plane
        planeLock = addComponent<LockToPlaneComponent>(vec3(0, 1, 0));

        health = addComponent<HealthComponent>(100.0f, 10.0f, 0);

        // Death subscription
        if (Services::eventBus) {
            Services::eventBus->process<DeathEvent>([this](const DeathEvent& e) {
                if (e.target == health.get()) {
                    for (auto hp : hardpoints)
                        hp->stopFiring();
                }
                });
        }
    }

    void initialize() {

        auto engineEmitter = std::make_shared<ParticleEmitter3D>();
        this->engineEmitter = engineEmitter;
        engineEmitter->particleTemplate = std::make_shared<Sprite3D>("smoke", Sprite3D::Mode::Billboard);
		engineEmitter->particleTemplate->scale = vec3(4.0f);
        engineEmitter->particleParent = this->parent;
		engineEmitter->inheritParentVelocity = true;
        engineEmitter->particleLifetime = 0.1f;
		engineEmitter->position = vec3(0, 0, 7.5f);
		engineEmitter->rotation = quat(vec3(0, glm::pi<float>(), 0));
        this->addChild(engineEmitter);
		Services::updateSystem->addNode(engineEmitter);


        for (const auto child : children) {

            if (auto casted = std::dynamic_pointer_cast<Model3D>(child)) {
                body = casted;
            }
        }
	}


    // ---------------- Main Update ----------------
    void update(double dt) override {
        for (auto& hp : hardpoints) hp->update(dt);

        if (!isDead()) {
            vec2 thrust = applyThrust(thrustDir, dt);
            float torque = applyRotationThrust(targetRot, thrustDir, dt);
			emitParticleEffect(glm::length(thrust));

            float delta = dt;
            
            auto vel = vec2(physics->velocity.x, physics->velocity.z);
            auto speed = glm::length(vel);

            rollSpeed -= glm::sin(roll) * centering * delta;
            rollSpeed -= glm::cos(0.8 * roll) * physics->angularVelocity.y * spinFactor * (max(0.0f, speed - 8.0f )/ 20.0f) * delta;

            rollSpeed -= glm::cos(0.8 * roll) * torque * delta;


            rollSpeed *= (1.0 - damping * delta);

            roll = roll + rollSpeed * delta;

            roll = glm::clamp(roll, -glm::half_pi<float>() * 1.1f, glm::half_pi<float>() * 1.1f);
            


            auto bodyPtr = body.lock();
            if (bodyPtr) {
                bodyPtr->rotation = glm::angleAxis(roll, vec3(0,0,-1));
            }


            //aero


            auto f = right();            
            auto rightDir = normalize(vec2(f.x, f.z));
            float perpendicular = dot(vel, rightDir);
            

            applyForce(-f * speed * perpendicular * fabs(sin(roll)) * 0.2f, dt);
            
        }

        // Update physics + all components
        PhysicalActor3D::update(dt);

        if (screenMin != screenMax)
            borderCollision(screenMin, screenMax, 0.5f, -0.06f);

        markDirty();
    }

    void setThrust(const vec2& input) {
        thrustDir = (length(input) > 1.0f) ? normalize(input) : input;
    }

    void setDirection(const vec2& dir) {
        targetRot = (length(dir) > 0.05f) ? normalize(dir) : vec2(0.0f);
    }

    // IControllable
    void setMoveDirection(const vec2 dir) override { setThrust(dir); }
    void setAimDirection(const vec2 dir) override { setDirection(dir); }

    void useAbility(int index, bool pressed) override {
        if (isDead()) return;
        if (index < 0 || index >= 4) return;

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

    // ---------------- Health Helpers ----------------
    bool isDead() const { return health->isDead(); }
    void applyDamage(float amount, void* source = nullptr) { health->applyDamage(amount, source); }
    void heal(float amount) { health->heal(amount); }
    void respawnHealth() { health->respawn(); }

    void respawn(vec3 pos, float rot = 0.0f) {
        position = pos;
        respawnHealth();
        resetPhysics();
        markDirty();
    }

    void addHardpoint(const std::shared_ptr<Hardpoint3D>& hp, int actionGroup) {
        hardpoints.push_back(hp);
        addChild(hp);
        actionMap[actionGroup].push_back(hp);
    }

private:
    // ---------------- Planar Helpers ----------------
    vec2 planarForward() const {
        vec3 f = forward();           // ship's forward vector in world space
        return normalize(vec2(f.x, f.z)); // XZ plane
    }

    vec2 getVelocityPlane() const {
        vec3 v = getVelocity();
        return vec2(v.x, v.z);
    }

    vec3 getAngularVelocityPlane() const {
        vec3 av = getAngularVelocity();
        return vec3(0.0f, av.y, 0.0f);
    }

    // ---------------- Corrected rotation ----------------
    float applyRotationThrust(const vec2& targetDir, const vec2& thrustDir, double dt) {
        // use targetDir if set, otherwise thrustDir
        vec2 dir = (length(targetDir) > 0.001f) ? targetDir : thrustDir;
        if (length(dir) < 0.001f) return 0.0f;


        vec2 forwardDir = planarForward(); // current forward in XZ-plane
        

		float angleDiff = orientedAngle(dir, forwardDir);



        float torque = PD_p * angleDiff - PD_d * getAngularVelocityPlane().y;
        torque = clamp(torque, -rotationThrust, rotationThrust);

        applyTorque(vec3(0, torque, 0), dt);
        
        return torque;
    }

    // ---------------- Corrected thrust ----------------
    vec3 applyThrust(const vec2& direction, double dt) {
        if (length(direction) < 0.01f) return vec3(0.0f);

        vec2 thrustDirNorm = normalize(direction);
        vec2 forwardDir = planarForward();

        // Map input X/Y → X/Z world
        vec3 thrustWorld3D(thrustDirNorm.x * baseThrust, 0, thrustDirNorm.y * baseThrust);

        float speed = length(getVelocityPlane());
        float totalMultiplier = 1.0f;

        float forwardAlignment = dot(thrustDirNorm, forwardDir);
        if (forwardAlignment > 0.0f)
            totalMultiplier += forwardAlignment * bonusThrustMultiplier;

        if (speed > 0.001f) {
            vec2 velDir = normalize(getVelocityPlane());
            float brakingAlignment = dot(thrustDirNorm, -velDir);
            if (brakingAlignment > 0.0f)
                totalMultiplier += brakingAlignment * bonusBrakingMultiplier * max(0.0f, forwardAlignment);
        }

        vec3 finalThrust = thrustWorld3D * totalMultiplier;

        applyForce(finalThrust, dt);

        return finalThrust; // just for reference
    }


    void borderCollision(const vec2& screenMin, const vec2& screenMax, float restitution = 0.5f, float spinFactor = 0.3f) {
        vec3 vel = physics->velocity;
        float angVelY = physics->angularVelocity.y;

        if (position.x < screenMin.x) { position.x = screenMin.x; if (vel.x < 0.0f) { vel.x = -vel.x * restitution; angVelY += vel.z * spinFactor; } }
        else if (position.x > screenMax.x) { position.x = screenMax.x; if (vel.x > 0.0f) { vel.x = -vel.x * restitution; angVelY -= vel.z * spinFactor; } }

        if (position.z < screenMin.y) { position.z = screenMin.y; if (vel.z < 0.0f) { vel.z = -vel.z * restitution; angVelY -= vel.x * spinFactor; } }
        else if (position.z > screenMax.y) { position.z = screenMax.y; if (vel.z > 0.0f) { vel.z = -vel.z * restitution; angVelY += vel.x * spinFactor; } }

        physics->velocity = vel;
        physics->angularVelocity.y = angVelY;
    }

    void emitParticleEffect(float thrust) {
        if (auto emitter = engineEmitter.lock()) {
            if(thrust/baseThrust < 0.1) {
                emitter->emitRate = 0.0f;
                return;
			}
			emitter->initialSpeed = (0.3 * baseThrust) * (thrust / baseThrust);
			emitter->emitRate = 70.0f * (thrust / baseThrust);
        }
	}
};