#pragma once


class BaseComponent {
public:
    virtual ~BaseComponent() = default;
    virtual void update(double dt) {}
};