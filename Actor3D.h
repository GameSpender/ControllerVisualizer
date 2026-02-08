#pragma once
#include "Transform3D.h"
#include "EventBus.h"
#include "InputSystem.h"
#include <string>
#include <typeindex>
#include "BaseComponent3D.h"

class Actor3D : public Transform3D {
public:
    std::vector<std::shared_ptr<BaseComponent3D>> components;
	bool expired = false; // for cleanup

    Actor3D() = default;
    virtual ~Actor3D() = default;

    // ---------------- Component System ----------------
    template <typename T, typename... Args>
    std::shared_ptr<T> addComponent(Args&&... args) {
        static_assert(std::is_base_of<BaseComponent3D, T>::value,
            "T must inherit BaseComponent");

        auto comp = std::make_shared<T>(std::forward<Args>(args)...);
        comp->owner = this;
        components.push_back(comp);
        return comp;
    }

    template <typename T>
    void removeComponent() {
        components.erase(
            std::remove_if(
                components.begin(),
                components.end(),
                [&](const std::shared_ptr<BaseComponent3D>& c) {
                    return dynamic_cast<T*>(c.get()) != nullptr;
                }),
            components.end()
        );
    }

    template <typename T>
    std::shared_ptr<T> getComponent() {
        for (auto& c : components) {
            if (auto t = std::dynamic_pointer_cast<T>(c))
                return t;
        }
        return nullptr;
    }

    // ---------------- Update ----------------
    void update(double dt) {
        std::sort(components.begin(), components.end(),
            [](const std::shared_ptr<BaseComponent3D>& a,
                const std::shared_ptr<BaseComponent3D>& b) {
                    return a->priority < b->priority;
            });

        for (auto& comp : components) {
            if (comp->enabled)
                comp->update(dt);
        }
    }

    // ---------------- Direction Helpers ----------------
    // Local-space axes
    vec3 forward() const { return normalize(rotation * vec3(0, 0, -1)); }
    vec3 up()      const { return normalize(rotation * vec3(0, 1, 0)); }
    vec3 left()   const { return normalize(rotation * vec3(1, 0, 0)); }

    // World-space axes
    vec3 forwardWorld() { return normalize(getWorldRotation() * vec3(0, 0, -1)); }
    vec3 upWorld()      { return normalize(getWorldRotation() * vec3(0, 1, 0)); }
    vec3 leftWorld()   { return normalize(getWorldRotation() * vec3(1, 0, 0)); }
};
