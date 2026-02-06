#pragma once
#include "Weapon.h"



class LaserGun : public Weapon {
    bool firing = false;

public:
    LaserGun() {
        shotInterval = 0.1f;
        damage = 25.0f;
        team = 0;
    }

    float shotSpeed = 4000.0f;

    float lifetime = 3.0f;


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
                team
            );
            projectile->init();
            projectile->scale = getWorldScale() / 1.5f;
            Services::projectiles->addProjectile(projectile);

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

class LaserMinigun : public Weapon {
    bool firing = false;
    float currentHeat = 0.0f;
    bool overheated = false;
    float spoolTimeRemaining = 0.0f; // seconds left to fully spool
    bool spoolSoundPlaying = false;  // track if spool sound is active
    bool shootSoundPlaying = false;
    bool stoppedFiring = true;

public:
    LaserMinigun() {
        shotInterval = 60.0f / 6000.0f;
        damage = 8.0f;
        team = 0;
        spoolTimeRemaining = spool; // start fully unspooled
        deviation = radians(0.6f);
        recoil = 1.0f;
    }

    // Minigun properties
    float shotSpeed = 7000.0f;
    float lifetime = 2.0f;

    // Heat / spool system
    float spool = 0.8f;        // seconds to spool
    float spoolDown = 1.0f;    // seconds per second to spool down
    float heatPerShot = 0.1f;
    float maxHeat = 100.0f;
    float dissipation = 15.0f;


    void startFiring() override {
        firing = true;

        // Start spool sound if not already and gun not overheated
        if (!spoolSoundPlaying && !overheated) {
            float startTime = spool - spoolTimeRemaining; // 0 = unspooled, 1 = fully spooled
            if (Services::eventBus)
                Services::eventBus->emit(SoundEvent{
                    .owner = &spoolSoundPlaying,
                    .soundName = "minigun_spool",
                    .startTime = startTime
                    });
            spoolSoundPlaying = true;
        }
    }

    void stopFiring() override {
        firing = false;

        // Stop spool sound
        if (spoolSoundPlaying && Services::eventBus) {
            Services::eventBus->emit(SoundEvent{
                .owner = &spoolSoundPlaying,
                .soundName = "minigun_spool",
                .stop = true
                });
            spoolSoundPlaying = false;
        }
        if (shootSoundPlaying && Services::eventBus) {
            Services::eventBus->emit(SoundEvent{
                .owner = &stoppedFiring,
                .soundName = "minigun_stop"
                });
            Services::eventBus->emit(SoundEvent{
                .owner = &shootSoundPlaying,
                .soundName = "minigun_shoot",
                .stop = true
                });
            shootSoundPlaying = false;
        }
    }

    void update(double dt) override {
        nextShot -= dt;

        // Dissipate heat
        if (currentHeat > 0.0f)
            currentHeat = std::max(0.0f, currentHeat - dissipation * (float)dt);

        // Recover from overheat if cooled
        if (overheated && currentHeat == 0.0f)
            overheated = false;

        // Handle firing logic
        if (!firing || overheated) {
            // Spool down when not firing
            spoolTimeRemaining = std::min(spool, spoolTimeRemaining + spoolDown * (float)dt);

            // Stop spool sound if it was playing
            if (spoolSoundPlaying && Services::eventBus) {
                Services::eventBus->emit(SoundEvent{
                    .owner = &spoolSoundPlaying,
                    .soundName = "minigun_spool",
                    .stop = true
                    });
                spoolSoundPlaying = false;
            }
            return;
        }

        // Spool up
        if (spoolTimeRemaining > 0.0f) {
            spoolTimeRemaining -= (float)dt;
            spoolTimeRemaining = std::max(0.0f, spoolTimeRemaining);


            return;
        }

        // fully spooled, make funny sounds
        if (spoolSoundPlaying && Services::eventBus) {
            Services::eventBus->emit(SoundEvent{
                .owner = &spoolSoundPlaying,
                .soundName = "minigun_spool",
                .stop = true
                });
            spoolSoundPlaying = false;
        }
        if (!shootSoundPlaying && Services::eventBus) {
            Services::eventBus->emit(SoundEvent{
            .owner = &shootSoundPlaying,
            .soundName = "minigun_shoot",
            .loop = true
                });
            shootSoundPlaying = true;
        }

        // Gun fully spooled, check overheat
        if (currentHeat >= maxHeat) {
            overheated = true;

            // Stop spool sound
            if (spoolSoundPlaying && Services::eventBus) {
                Services::eventBus->emit(SoundEvent{
                    .owner = &spoolSoundPlaying,
                    .soundName = "minigun_spool",
                    .stop = true
                    });
                spoolSoundPlaying = false;

            }
            if (shootSoundPlaying && Services::eventBus) {
                Services::eventBus->emit(SoundEvent{
                    .owner = &stoppedFiring,
                    .soundName = "minigun_stop"
                    });
                Services::eventBus->emit(SoundEvent{
                .owner = &shootSoundPlaying,
                .soundName = "minigun_shoot",
                .stop = true
                    });
                shootSoundPlaying = false;
            }


            return;
        }

        // Fire projectile
        if (nextShot <= 0) {
            nextShot = shotInterval;
            currentHeat += heatPerShot;

            fireProjectile();
        }
    }

    void sendProjectileImpulse(vec2 intensity, float rotation) {
        auto par = parent.lock();
        if (auto receiver = dynamic_cast<PhysicsReceiver*>(par.get())) {
            receiver->applyImpulse(intensity);
            receiver->applyAngularImpulse(rotation);
        }
    }

    void fireProjectile() {
        auto parentPtr = parent.lock();
        if (!parentPtr) return;

        // Spawn projectile
        // Get the base forward direction
        vec2 dir = forwardWorld();

        // Apply random deviation
        static std::random_device rd;
        static std::mt19937 gen(rd());
        std::normal_distribution<float> dist(0.0f, deviation); // deviation in radians

        float angleOffset = dist(gen);

        // Rotate the direction vector by the angleOffset
        float cosA = cos(angleOffset);
        float sinA = sin(angleOffset);
        vec2 deviatedDir = vec2(
            dir.x * cosA - dir.y * sinA,
            dir.x * sinA + dir.y * cosA
        );


        // --- Compute recoil impulse ---
        float recoilMagnitude = damage * recoil / 1000.0f * shotSpeed; // scale as needed
        vec2 recoilImpulse = -deviatedDir * recoilMagnitude; // opposite to shot

        // --- Compute rotational impulse ---
        // Use perpendicular to shot vector relative to ship forward
        vec2 right = vec2(-dir.y, dir.x); // right-hand perpendicular
        float angularImpulse = dot(recoilImpulse, right); // 0.1 = scaling factor for rotation
        sendProjectileImpulse(recoilImpulse, angularImpulse);

        // Spawn projectile with deviated direction
        auto projectile = std::make_shared<LaserProjectile>(getWorldPosition(), deviatedDir * shotSpeed, lifetime, damage, team);
        projectile->init();
        projectile->scale = getWorldScale();
        projectile->spriteName = "bullet_shot";
        Services::projectiles->addProjectile(projectile);
    }
};


class EnemyGun : public Weapon {
    bool firing = false;

public:
    EnemyGun() {
        shotInterval = 1.0f;
        damage = 25.0f;
        team = 1;
    }

    float shotSpeed = 700.0f;

    float lifetime = 7.0f;


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
                team
            );
            projectile->init();
            projectile->scale = getWorldScale() / 1.5f;
            projectile->spriteName = "enemy_shot";
            Services::projectiles->addProjectile(projectile);

            Services::eventBus->emit(ShootEvent{
                .position = getWorldPosition(),
                .direction = forwardWorld(),
                .projectileType = "enemy_shot",
                .soundName = "enemy_shot",
                .effectName = "enemy_shot"
                });
        }
    }
};