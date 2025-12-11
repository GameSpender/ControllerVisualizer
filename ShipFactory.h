#pragma once
#include "Ship.h"
#include "Services.h"
#include "Guns.h"
#include <memory>
#include <glm/glm.hpp>

class ShipFactory {
public:

    // Spawn a generic ship at a position with optional scale and sprite
    static std::shared_ptr<Ship> spawnShip(
        const glm::vec2& position,
        float rotation = 0.0f,
        const glm::vec2& scale = glm::vec2(50.0f),
        const std::string& spriteName = "ship",
        int collisionLayer = CollisionLayer::Enemy, // default enemy layer
		const float colliderScale = 0.8f
    ) {
        auto ship = std::make_shared<Ship>();

        ship->spriteName = spriteName;
        ship->scale = scale;
        ship->respawn(position, rotation);

        // Screen bounds for AI/player ships
        ship->screenMin = glm::vec2(0.0f);
        ship->screenMax = glm::vec2(1920.0f, 1080.0f); // optionally configurable

        // Collider
        auto collider = std::make_shared<Collider2D>(Collider2D::ShapeType::Circle);
        collider->layer = collisionLayer;
        collider->mask = CollisionLayer::All; // collide with everything
        collider->scale = glm::vec2(colliderScale);
        ship->addChild(collider);
        Services::collisions->addCollider(collider);

        return ship;
    }

    // Convenience for spawning AI enemy ships
    static std::shared_ptr<Ship> spawnEnemy(
        const glm::vec2& position,
        float rotation = 0.0f,
        const glm::vec2& scale = glm::vec2(70.0f),
        const std::string& spriteName = "enemy_ship_basic"
    ) {
        auto enemyShip = spawnShip(position, rotation, scale, spriteName, CollisionLayer::Enemy, 1.0f);

        // --- Add a single hardpoint with EnemyGun bound to action 0 ---
        auto gun = std::make_shared<EnemyGun>();
        auto hardpoint = std::make_shared<Hardpoint>();
        hardpoint->attachWeapon(gun);
        enemyShip->addHardpoint(hardpoint, 0);

        return enemyShip;
    }

    // Convenience for spawning player ships
    static std::shared_ptr<Ship> spawnPlayer(
        const glm::vec2& position,
        float rotation = 0.0f,
        const glm::vec2& scale = glm::vec2(50.0f),
        const std::string& spriteName = "ship"
    ) {
        return spawnShip(position, rotation, scale, spriteName, CollisionLayer::Player);
    }
};
