#pragma once
#include "Transform2D.h"
#include "EventBus.h"
#include "InputSystem.h"
#include <string>
#include <typeindex>
#include "BaseComponent.h"


class Actor2D : public Transform2D {
public:
    std::vector<std::shared_ptr<BaseComponent>> components;
    std::string spriteName;

    Actor2D() = default;
    virtual ~Actor2D() = default;

    template <typename T, typename... Args>
    std::shared_ptr<T> addComponent(Args&&... args) {
        static_assert(std::is_base_of<BaseComponent, T>::value,
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
                [&](const std::shared_ptr<BaseComponent>& c) {
                    return dynamic_cast<T*>(c.get()) != nullptr;
                }),
            components.end()
        );
    }

    template <typename T>
    std::shared_ptr<T> getComponent(){
        for (auto& c : components) {
            if (auto t = std::dynamic_pointer_cast<T>(c))
                return t;
        }
        return nullptr;
    }

    // Pure virtual update
    virtual void update(double dt) {};


    vec2 forward() {
        float rot = -rotation - radians(90.0f);
        return vec2(cos(rot), sin(rot));
    }

    vec2 forwardWorld() {
        float rot = - getWorldRotation() - radians(90.0f);
        return vec2(cos(rot), sin(rot));
    }
};


