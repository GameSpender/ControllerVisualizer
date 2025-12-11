#pragma once
#include "Transform2D.h"
#include "EventBus.h"
#include "InputSystem.h"
#include <string>

class Actor2D : public Transform2D {
protected:

public:
    std::string spriteName;

    // Constructor takes pointers now
    Actor2D() = default;

    virtual ~Actor2D() = default;

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


