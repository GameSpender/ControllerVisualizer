#pragma once
#include "Transform2D.h"
#include "EventBus.h"
#include "InputSystem.h"
#include <string>

class Actor2D : public Transform2D {
protected:
    PlayerInput& input;
    EventBus& events;

public:

    std::string spriteName;

    Actor2D(PlayerInput& input, EventBus& events)
        : input(input), events(events) {
    }

    virtual void update(double dt) = 0;


};
