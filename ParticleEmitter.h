#pragma once
#include "Actor3D.h"
#include "PhysicalActor3D.h"
#include "LifetimeComponent.h"
#include "Sprite3D.h"
#include "Services.h"
#include "UpdateSystem.h"
#include <memory>
#include <random>

#include "Header.h">

#include <glm/gtx/string_cast.hpp>

class ParticleEmitter3D : public Actor3D {
private:
    float accumulator = 0.0f;

    std::default_random_engine rng{ std::random_device{}() };
    glm::vec3 inheritedVelocity;

public:
    std::shared_ptr<Sprite3D> particleTemplate;
    std::weak_ptr<Transform3D> particleParent;

    // Base emission rate (particles per second)
    float emitRate = 10.0f;

    // Base speed magnitude
    float initialSpeed = 10.0f;

    

    // Standard deviation of speed
    float velocityRandomness = 0.0f;

    // Angular randomness in radians (stddev)
    float angleRandomness = 0.0f;

    float particleLifetime = 2.0f;

	bool inheritParentVelocity = false;

	ParticleEmitter3D() = default;

    ParticleEmitter3D(std::shared_ptr<Sprite3D> spriteTemplate,
        std::weak_ptr<Transform3D> parent)
        : particleTemplate(spriteTemplate),
        particleParent(parent)
    {
    }

    void update(double dt) override
    {
        float currentRate = glm::max(0.0f, emitRate);
        accumulator += dt * currentRate;

        while (accumulator >= 1.0f) {


            auto origin = parent.lock();
            if (inheritParentVelocity && origin)
            {
				auto phys = std::dynamic_pointer_cast<PhysicalActor3D>(origin);
                if (phys) {
					auto parentVel = phys->getVelocity();
					inheritedVelocity = parentVel; // Convert to local space
                }
            }

			

            emitParticle();
            accumulator -= 1.0f;
        }

        Actor3D::update(dt);
    }

    void emitParticle()
    {
        if (!particleTemplate) return;

        // Clone sprite
        auto pSprite = std::make_shared<Sprite3D>(*particleTemplate);
        auto particle = std::make_shared<PhysicalActor3D>();
        
        particle->addChild(pSprite);
        particle->addComponent<LifetimeComponent>(particleLifetime);
        
		auto velocity = forward() * initialSpeed + inheritedVelocity;
        
        auto world = particleParent.lock();
		auto worldMatrix = mat4(1.0f);
        if (world) {
            addChild(particle);
            reparentKeepWorld(particle, world);
			worldMatrix = world->getWorldMatrix();
        }
        velocity = Physics::convertVelocityToNewWorld(velocity, getWorldMatrix(), worldMatrix);
        particle->physics->velocity = velocity;

		Services::renderSystem->submit(pSprite);
        Services::updateSystem->addNode(pSprite);
        Services::updateSystem->addNode(particle);
    }

private:

    // Sample normal distribution
    float sampleNormal(float mean, float stddev)
    {
        if (stddev <= 0.0f) return mean;
        std::normal_distribution<float> dist(mean, stddev);
        return dist(rng);
    }

    // Slight angular deviation using Gaussian
    glm::vec3 randomizeDirection(const glm::vec3& baseDir)
    {
        if (angleRandomness <= 0.0f)
            return glm::normalize(baseDir);

        std::normal_distribution<float> angleDist(0.0f, angleRandomness);

        float yaw = angleDist(rng);
        float pitch = angleDist(rng);

        glm::quat qYaw = glm::angleAxis(yaw, glm::vec3(0, 1, 0));
        glm::vec3 right = glm::normalize(glm::cross(baseDir, glm::vec3(0, 1, 0)));
        glm::quat qPitch = glm::angleAxis(pitch, right);

        glm::vec3 newDir = qYaw * qPitch * baseDir;
        return glm::normalize(newDir);
    }
};
