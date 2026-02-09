#pragma once
#include "Weapon3D.h"
#include "Projectile3D.h"
#include "UpdateSystem.h"
#include "Services.h"
#include "Sprite3D.h"
#include "RenderSystem.h"
#include "Light.h"



class LaserGun : public Weapon3D{
    bool firing = false;

public:
    LaserGun() {
        shotInterval = 0.1f;
        damage = 25.0f;
        team = 0;
    }

    float shotSpeed = 800.0f;

    float lifetime = 2.0f;


    void startFiring() override { firing = true; }
    void stopFiring() override { firing = false; }

    void update(double dt) override {
        nextShot -= dt;


        if (!firing) return;


        if (nextShot <= 0) {
            nextShot = shotInterval;


            auto parentPtr = parent.lock();
            if (!parentPtr) return;

            // spawn projectile

            auto projectile = std::make_shared<LaserProjectile>(
                getWorldPosition(),
                forwardWorld() * shotSpeed,
                lifetime,
                damage,
                team,
                shared_from_this()
            );
            projectile->init();
            projectile->scale = getWorldScale() / 1.5f;

            auto sprite = std::make_shared<Sprite3D>("laser_shot", Sprite3D::Mode::Billboard);
            sprite->setEmissiveColor(vec3(0.4f, 1.0f, 0.3f));
            sprite->rotation = glm::vec3(0, 0, glm::half_pi<float>());
            projectile->addChild(sprite);


            auto light = std::make_shared<PointLight3D>();
            light->color = vec3(0, 1, 0);
            light->intensity = 4.0f;
            light->range = 50.0f;
            projectile->addChild(light);
            
            Services::updateSystem->addNode(projectile);
            Services::renderSystem->submit(sprite);
            Services::lights->addLight(light);

            Services::eventBus->emit(ShootEvent{
                .position = getWorldPosition(),
                .direction = forwardWorld(),
                .projectileType = "laser_shot",
                .soundName = "laser_shot",
                .effectName = "laser_shot"
                });
        }
    }
};