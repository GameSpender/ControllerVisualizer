#pragma once
class Actor2D;

class BaseComponent {
public:
    Actor2D* owner = nullptr;

    virtual ~BaseComponent() = default;
    virtual void update(double dt) {}
};
