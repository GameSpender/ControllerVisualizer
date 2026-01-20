#pragma once

// Forward declarations
class InputSystem;
class EventBus;
class AssetManager;
class SoundManager;
class EventHandler;
class ProjectileSystem;
class CollisionSystem;

struct Services {

    // Core systems
    inline static InputSystem* inputSystem = nullptr;
    inline static EventBus* eventBus = nullptr;
    inline static AssetManager* assets = nullptr;
    inline static SoundManager* sound = nullptr;
    inline static EventHandler* eventHandler = nullptr;
    inline static ProjectileSystem* projectiles = nullptr;
	inline static CollisionSystem* collisions = nullptr;


    // Initialize everything
    static void init(
        InputSystem* inputSystem_,
        EventBus* eventBus_,
        AssetManager* assetManager_,
        SoundManager* soundManager_,
        EventHandler* eventHandler_,
        ProjectileSystem* projectileSystem_,
		CollisionSystem* collisionSystem_
    )
    {
        inputSystem = inputSystem_;
        eventBus = eventBus_;
        assets = assetManager_;
        sound = soundManager_;
        eventHandler = eventHandler_;
        projectiles = projectileSystem_;
		collisions = collisionSystem_;
    }
};
