#pragma once
class Actor3D;

class BaseComponent3D {
public:
    Actor3D* owner = nullptr;

    virtual ~BaseComponent3D() = default;

    // Called every frame
    virtual void update(double dt) {}
};
