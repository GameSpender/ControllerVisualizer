//#pragma once
//#include "Projectile.h"
//#include <vector>
//#include "AssetManager.h"
//
//class ProjectileSystem {
//public:
//    std::vector<std::shared_ptr<Projectile>> projectiles;
//
//    // Update all projectiles
//    void update(double dt) {
//        for (auto& p : projectiles)
//            p->update(dt);
//
//        removeDead();
//    }
//
//    // Draw all projectiles
//    void render(SpriteRenderer& renderer, AssetManager& assets) {
//        for (auto& p : projectiles) {
//            std::weak_ptr<Texture> tex = assets.getTexture(p->spriteName);
//			if (tex.expired()) continue; // Skip if texture not found
//            renderer.Draw(tex.lock()->id, p->getWorldMatrix());
//        }
//    }
//
//    // Add any projectile type
//    void addProjectile(const std::shared_ptr<Projectile>& proj) {
//        projectiles.push_back(proj);
//    }
//
//private:
//    void removeDead() {
//        projectiles.erase(
//            std::remove_if(projectiles.begin(), projectiles.end(),
//                [](auto& p) { return p->isDead(); }),
//            projectiles.end()
//        );
//    }
//};
