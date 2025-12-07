#pragma once
#include "SpriteRenderer.h"
#include "InteractionInterfaces.h"
#include <glm/glm.hpp>
#include <vector>
#include "Actor.h"

using namespace glm;

class Projectile : public Actor2D{
public:
    vec2 velocity;         // world-space velocity
    float lifetime; // seconds
    int team;

    Projectile() {}

    bool isDead() const { return lifetime <= 0.0f; }
};


class LaserProjectile : public Projectile {
public:
    float damage = 0.0f;

    // Constructor with all needed parameters
    LaserProjectile(const glm::vec2& startPos, const glm::vec2& velocity, float lifetime, float damage, int team)
    {
        this->position = startPos;  // inherited from Transform2D
        this->velocity = velocity;
        this->setRotation(velocity);
        this->lifetime = lifetime;
        this->damage = damage;
        this->team = team;
        this->spriteName = "laser_shot"; // optional, can be set externally
    }

    // Update moves the projectile and decreases lifetime
    void update(double dt) override {
        position += velocity * static_cast<float>(dt);
        lifetime -= static_cast<float>(dt);

        markDirty();
    }
};

//class Explosion : public Projectile {
//public:
//    float explodeSize = 5.0f;
//    float rotationSpeed = 0.0f;
//    float initial_lifetime;
//    float initial_size;
//
//    Explosion() {}
//
//    void update(double dt) override {
//        position += velocity * dt;
//        velocity -= velocity * friction * dt;
//        lifetime -= dt;
//
//        float t = 1.0f - (lifetime / initial_lifetime);
//        float eased = 1.0f - (1.0f - t) * (1.0f - t); // quad ease-out
//    
//        scale = vec2(mix(initial_size, explodeSize, eased));
//        rotation += rotationSpeed * dt;
//
//        markDirty();
//    }
//};
