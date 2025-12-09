#pragma once
#include "Transform2D.h"
#include "EventBus.h"
#include "InputSystem.h"
#include "CollisionSystem.h"
#include <string>

class Actor2D : public Transform2D {
protected:
    PlayerInput* input = nullptr;
    EventBus* events = nullptr;
    CollisionSystem* collisions = nullptr;

public:
    std::string spriteName;

    // Constructor takes pointers now
    Actor2D() = default;

    virtual ~Actor2D() = default;

    void init(PlayerInput* inputPtr = nullptr, EventBus* eventsPtr = nullptr, CollisionSystem* collisionsPtr = nullptr) {
        input = inputPtr;
        events = eventsPtr;
        collisions = collisionsPtr;
    }

    void init(PlayerInput* inputPtr) {
        input = inputPtr;
    }
    void init(EventBus* eventsPtr) {
        events = eventsPtr;
    }
    void init(CollisionSystem* collisionsPtr) {
        collisions = collisionsPtr;
    }

    // Pure virtual update
    virtual void update(double dt) {};

    // Optional helper functions to check systems
    bool hasInput() const { return input != nullptr; }
    bool hasEventBus() const { return events != nullptr; }

    vec2 forward() {
        float rot = -rotation - radians(90.0f);
        return vec2(cos(rot), sin(rot));
    }

    vec2 forwardWorld() {
        float rot = - getWorldRotation() - radians(90.0f);
        return vec2(cos(rot), sin(rot));
    }
};
