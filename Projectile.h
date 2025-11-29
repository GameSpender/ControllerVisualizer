#pragma once
#include "Transform2D.h"
#include "SpriteRenderer.h"
#include "InteractionInterfaces.h"
#include <glm/glm.hpp>
#include <vector>

using namespace glm;

class Projectile : public Transform2D, public Animated {
public:
    vec2 velocity;         // world-space velocity
    float lifetime = 3.0f; // seconds
    unsigned int texture;
    float radius = 5.0f;   // for simple collision
    float friction = 0.0f;

    Projectile(unsigned int tex, vec2 startPos, vec2 vel)
        : texture(tex), velocity(vel)
    {
        position = startPos;
    }

    Projectile(unsigned int tex, vec2 startPos, vec2 vel, float lifetime, float size, float rotation = 0.0f, float friction = 0.0f)
        : texture(tex), velocity(vel), lifetime(lifetime), friction(friction){
        position = startPos;
        scale = vec2(size);
        this->rotation = rotation;
    }

    void update(double dt) override {
        float delta = (float)dt;
        position += velocity * delta;
        velocity -= velocity * friction * delta;
        lifetime -= delta;
        markDirty();
    }

    void Draw(SpriteRenderer& renderer) override {
        renderer.Draw(texture, getWorldMatrix());
    }

    bool isDead() const { return lifetime <= 0.0f; }
};
