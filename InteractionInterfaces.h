#pragma once
#include <glm/glm.hpp>


class Updateable {
public:
    virtual void update(double dt) {}
};

class Interactive {
public:
    virtual Interactive* hitTest(glm::vec2 world) = 0;
    virtual void onMouseInput(int button, int inputType) {}
    virtual void onMouseMove(glm::vec2) {}

    virtual ~Interactive() = default;
};

class Animated {
public:
    virtual void update(double dt) {}
	virtual ~Animated() = default;
};