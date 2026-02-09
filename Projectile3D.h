#pragma once
#include "SpriteRenderer.h"
#include "PhysicalActor3D.h"
#include <glm/glm.hpp>
#include <vector>

#include "Services.h"
#include "Sprite3D.h"
#include "CollisionSystem3D.h"

#include "TeamRules.h"
#include "HealthComponent.h"
#include "LifetimeComponent.h"
#include "Light.h"

using namespace glm;

class Projectile : public PhysicalActor3D {
public:
    
	std::shared_ptr<LifetimeComponent> lifetimeComponent;
    std::weak_ptr<Transform3D> owner; // The originator of the projectile
    int team;


    Projectile() {
        lifetimeComponent = addComponent<LifetimeComponent>(3.0f); 
    }

    virtual void init() {}

    void makeCollider(float size) {
        auto collider = std::make_shared<Collider3D>(Collider3D::ShapeType3D::Sphere);
        collider->layer = CollisionLayer::Projectile;
        collider->mask = CollisionLayer::All - CollisionLayer::Projectile;
        collider->scale = vec3(0.2f);
        addChild(collider);

        // Set the collision callback
        collider->onCollisionEnter = [this](Collider3D* other) {
            // Call the virtual function
            this->hitSomething(other ? dynamic_cast<Transform3D*>(other->parent.lock().get()) : nullptr);
            };
        Services::collisions3D->addCollider(collider);


    }

    virtual void hitSomething(Transform3D* other) {
        if (other) {
            // Example: print or apply damage
            // printf("Hit actor with sprite: %s\n", other->spriteName.c_str());
        }
        // Default: destroy the projectile
		lifetimeComponent->lifetime = 0.0f;
        
    }
    bool isDead() const { return lifetimeComponent->lifetime <= 0.0f; }
};


class LaserProjectile : public Projectile {
public:
    float damage = 0.0f;
    float knockbackScale = 1.0f / 100.0f;



    // Constructor with all needed parameters
    LaserProjectile(vec3 startPos, vec3 velocity, float lifetime, float damage, int team, std::shared_ptr<Transform3D> owner)
    {
        this->position = startPos;  // inherited from Transform2D
        this->physics->velocity = velocity;
        this->rotation = velocity;
        this->lifetimeComponent->lifetime = lifetime;
        this->damage = damage;
        this->team = team;
        this->owner = owner; // assign the owner
    }

    void init() override {
        makeCollider(0.5f);
    }

    void hitSomething(Transform3D* other) {

        if (auto phys = dynamic_cast<PhysicalActor3D*>(other)) {
            phys->applyImpulse(getVelocity() * damage * knockbackScale);
        }

        if (auto dmg = dynamic_cast<Actor3D*>(other)->getComponent<HealthComponent>()) {

            // Team filter
            if (TeamRules::canDamage(this->team, dmg->team)) {

                dmg->applyDamage(damage, shared_from_this().get());
            }
        }

        lifetimeComponent->lifetime = 0.0f;
    }

    //// Update moves the projectile and decreases lifetime
    //void update(double dt) override {
    //    
    //    PhysicalActor3D::update()
    //}
};

