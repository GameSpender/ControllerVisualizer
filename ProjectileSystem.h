#pragma once
#include "Projectile.h"
#include <vector>
#include "AssetManager.h"

class ProjectileSystem {
public:
    std::vector<std::shared_ptr<Projectile>> projectiles;


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
    std::shared_ptr<T> addProjectile(Args&&... args) {
        auto proj = std::make_shared<T>(std::forward<Args>(args)...);
        projectiles.push_back(proj);
        return proj;
    }

    template<typename T, typename... Args>
    std::shared_ptr<T> spawnProjectile(Args&&... args) {
        return addProjectile<T>(std::forward<Args>(args)...);
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
