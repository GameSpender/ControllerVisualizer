#pragma once

// Forward declarations
class InputSystem;
class EventBus;
class AssetManager;
class SoundManager;
class EventHandler;
class ProjectileSystem;
class CollisionSystem;
class CollisionSystem3D;
class LightManager;
class RenderSystem;
class UpdateSystem;

struct Services {

    // Core systems
    inline static InputSystem* inputSystem = nullptr;
    inline static EventBus* eventBus = nullptr;
    inline static AssetManager* assets = nullptr;
	inline static LightManager* lights = nullptr;
    inline static SoundManager* sound = nullptr;
    inline static EventHandler* eventHandler = nullptr;
    inline static ProjectileSystem* projectiles = nullptr;
	inline static CollisionSystem* collisions = nullptr;
	inline static CollisionSystem3D* collisions3D = nullptr;
	inline static RenderSystem* renderSystem = nullptr;
	inline static UpdateSystem* updateSystem = nullptr;


    // Initialize everything
    static void init(
        InputSystem* inputSystem_,
        EventBus* eventBus_,
        AssetManager* assetManager_,
        SoundManager* soundManager_,
        EventHandler* eventHandler_,
        ProjectileSystem* projectileSystem_,
		CollisionSystem* collisionSystem_,
		CollisionSystem3D* collisionSystem3D_,
		LightManager* lightManager_,
		RenderSystem* renderSystem_,
		UpdateSystem* updateSystem_
    )
    {
        inputSystem = inputSystem_;
        eventBus = eventBus_;
        assets = assetManager_;
        sound = soundManager_;
        eventHandler = eventHandler_;
        projectiles = projectileSystem_;
		collisions = collisionSystem_;
		collisions3D = collisionSystem3D_;
		lights = lightManager_;
		renderSystem = renderSystem_;
		updateSystem = updateSystem_;
    }
};
