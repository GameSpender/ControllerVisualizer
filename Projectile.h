#pragma once
#include "SpriteRenderer.h"
#include "InteractionInterfaces.h"
#include "Physics.h"
#include <glm/glm.hpp>
#include <vector>
#include "Actor.h"

using namespace glm;

class Projectile : public Actor2D{
public:
    vec2 velocity;         // world-space velocity
    float lifetime; // seconds
    int team;

   

    Projectile(){}

    void init(CollisionSystem* coll) {
        Actor2D::init(coll);

        if (!collisions) return;
        std::shared_ptr<Collider2D> collider = std::make_shared<Collider2D>(Collider2D::ShapeType::Circle);
        addChild(collider);
        collider->layer = CollisionLayer::Projectile;
        collider->mask = CollisionLayer::All - CollisionLayer::Projectile;

        // Set the collision callback
        collider->onCollisionEnter = [this](Collider2D* other) {
            // Call the virtual function
            this->hitSomething(other ? dynamic_cast<Transform2D*>(other->parent.lock().get()) : nullptr);
            };

        collisions->addCollider(collider);
    }

    virtual void hitSomething(Transform2D* other) {
        if (other) {
            // Example: print or apply damage
            // printf("Hit actor with sprite: %s\n", other->spriteName.c_str());
        }
        // Default: destroy the projectile
        lifetime = 0.0f;
    }
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

    void hitSomething(Transform2D* other) {
        if (auto hit = dynamic_cast<PhysicsReceiver*>(other)) {
            hit->applyImpulse(velocity * damage / 100.0f);
        }
        lifetime = 0.0f;
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
