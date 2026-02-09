#pragma once
#include <memory>
#include <vector>
#include <algorithm>
#include "Light.h"

// -------------------------
// Manages weak references to lights
// -------------------------
class LightManager {
public:
    LightManager() = default;

	vec3 ambientColor = vec3(0.04f); // default ambient light

    // Add a light (weak_ptr)
    void addLight(const std::shared_ptr<Light3D>& light) {
        if (!light) return;
        lights.push_back(light);
    }

    // Remove a light explicitly
    void removeLight(const std::shared_ptr<Light3D>& light) {
        lights.erase(
            std::remove_if(lights.begin(), lights.end(),
                [&light](const std::weak_ptr<Light3D>& w) {
                    auto sp = w.lock();
                    return !sp || sp == light;
                }),
            lights.end()
        );
    }

    // Get all valid lights as shared_ptrs
    std::vector<std::shared_ptr<Light3D>> getActiveLights() {
        std::vector<std::shared_ptr<Light3D>> result;
        for (auto it = lights.begin(); it != lights.end();) {
            if (auto sp = it->lock()) {
                result.push_back(sp);
                ++it;
            }
            else {
                // remove expired
                it = lights.erase(it);
            }
        }
        return result;
    }

private:
    std::vector<std::weak_ptr<Light3D>> lights;
};
