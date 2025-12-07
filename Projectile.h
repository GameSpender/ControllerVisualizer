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
    float lifetime = 3.0f; // seconds
    unsigned int texture;
    float radius = 5.0f;   // for simple collision
    float friction = 0.0f;

    Projectile() {}

    virtual void update(double dt) override {
        float delta = (float)dt;
        position += velocity * delta;
        velocity -= velocity * friction * delta;
        lifetime -= delta;
        markDirty();
    }

    bool isDead() const { return lifetime <= 0.0f; }
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
