#pragma once
#include "BaseComponent.h"
#include "glm/glm.hpp"
#include "Services.h"
#include "EventBus.h"
#include <string>

class HealthComponent : public BaseComponent {
public:
    float health = 100.f;
    float maxHealth = 100.f;
    float armor = 0.f;
    int team = 0;

    HealthComponent() = default;
    HealthComponent(float hp, float ar, int t)
        : health(hp), maxHealth(hp), armor(ar), team(t) {
    }

    // Apply damage with optional source
    void applyDamage(float amount, void* source = nullptr) {
        if (!owner) return;

        if (health < 0) return;

        float actual = glm::max(0.0f, amount - armor);
        if (actual <= 0.0f) return;

        health -= actual;

        if (Services::eventBus) {
            DamageEvent e;
            e.target = owner;
            e.amount = actual;
            e.team = team;

            Services::eventBus->emit<DamageEvent>(e);
        }

        // Emit death event if health drops to zero
        if (health <= 0.0f) {
            health = 0.0f;

            if (Services::eventBus) {
                DeathEvent e;
                e.target = owner;
                e.team = team;

                Services::eventBus->emit<DeathEvent>(e);
            }
        }
    }

    // Heal the component
    void heal(float amount) {
        if (amount > 0.0f) {
            health += amount;
            if (health > maxHealth) health = maxHealth;
        }
    }

    // Check if dead
    bool isDead() const {
        return health <= 0.0f;
    }

    void respawn() {
        if (!owner) return;

        health = maxHealth;

        if (Services::eventBus) {
            RespawnEvent e;
            e.target = owner;
            e.team = team;
            Services::eventBus->emit<RespawnEvent>(e);
        }
    }

    void setTeam(int t) { team = t; }
    void setMaxHealth(float maxHp) { maxHealth = maxHp; }

    // Optional: override update if needed
    void update(double dt) override {
        // Could implement regen over time or other effects
    }
};
