#pragma once
#include <glm/glm.hpp>

class IControllable {
public:
	int abilityCount = 4;

    virtual ~IControllable() {}

    virtual glm::vec2 getPostion() = 0;
    // Movement
    virtual void setMoveDirection(const glm::vec2 dir) = 0;

    // Aiming / facing
    virtual void setAimDirection(const glm::vec2 dir) = 0;

    // Abilities 1-4
    virtual void useAbility(int index, bool pressed) = 0;

    // Dash or similar movement ability
    virtual void dash(bool pressed) = 0;
};
