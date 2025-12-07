#pragma once
#include "Projectile.h"
#include <vector>
#include "AssetManager.h"

class ProjectileSystem {
public:
    std::vector<std::unique_ptr<Projectile>> projectiles;

    void update(double dt) {
        for (auto& p : projectiles)
            p->update(dt);

        removeDead();
    }

    void render(SpriteRenderer& renderer, AssetManager& assets) {
        for (auto& p : projectiles) {
            auto* tex = assets.getTexture(p->spriteName);
            renderer.Draw(tex->id, p->getWorldMatrix());
        }
    }

    template<typename T, typename... Args>
    void addProjectile(Args&&... args) {
        projectiles.push_back(std::make_unique<T>(std::forward<Args>(args)...));
    }

    // Optional helper to create and add in-place
    template <typename T, typename... Args>
    void spawnProjectile(Args&&... args) {
        auto proj = std::make_unique<T>(std::forward<Args>(args)...);
        projectiles.push_back(std::move(proj));
    }

private:
    void removeDead() {
        projectiles.erase(
            std::remove_if(projectiles.begin(), projectiles.end(),
                [](auto& p) { return p->isDead(); }),
            projectiles.end()
        );
    }
};
