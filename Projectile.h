#pragma once
#include "SpriteRenderer.h"
#include "InteractionInterfaces.h"
#include "PhysicalActor2D.h"
#include <glm/glm.hpp>
#include <vector>
#include "Actor.h"

#include "Collider.h"
#include "CollisionSystem.h"
#include "Services.h"

#include "TeamRules.h"
#include "HealthComponent.h"

using namespace glm;

class Projectile : public Actor2D{
public:
    Transform2D* owner = nullptr; // The originator of the projectile
    vec2 velocity;         // world-space velocity
    float lifetime; // seconds
    int team;

   

    Projectile() = default;

	virtual void init() {}

    void makeCollider(float size) {
        auto collider = std::make_shared<Collider2D>(Collider2D::ShapeType::Circle);
        collider->layer = CollisionLayer::Projectile;
        collider->mask = CollisionLayer::All - CollisionLayer::Projectile;
        collider->scale = vec2(0.2f);
        addChild(collider);

        // Set the collision callback
        collider->onCollisionEnter = [this](Collider2D* other) {
            // Call the virtual function
            this->hitSomething(other ? dynamic_cast<Transform2D*>(other->parent.lock().get()) : nullptr);
            };
        Services::collisions->addCollider(collider);
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
    float knockbackScale = 1.0f / 100.0f;

    // Constructor with all needed parameters
    LaserProjectile(vec2 startPos, vec2 velocity, float lifetime, float damage, int team, Transform2D* owner = nullptr)
    {
        this->position = startPos;  // inherited from Transform2D
        this->velocity = velocity;
        this->setRotation(velocity);
        this->lifetime = lifetime;
        this->damage = damage;
        this->team = team;
        this->owner = owner; // assign the owner
        this->spriteName = "laser_shot"; // optional, can be set externally
    }
     
    void init() override {
        makeCollider(0.5f);
    }

    void hitSomething(Transform2D* other) {

        if (auto phys = dynamic_cast<PhysicalActor2D*>(other)) {
            phys->applyImpulse(velocity * damage * knockbackScale);
        }
        
        if (auto dmg = dynamic_cast<Actor2D*>(other)->getComponent<HealthComponent>()) {

            // Team filter
            if (TeamRules::canDamage(this->team, dmg->team)) {

                dmg->applyDamage(damage, this->owner);
            }
        }

        lifetime = 0;
    }

    // Update moves the projectile and decreases lifetime
    void update(double dt) override {
        position += velocity * static_cast<float>(dt);
        lifetime -= static_cast<float>(dt);

        markDirty();
    }
};

