#pragma once
#include "Transform2D.h"
#include <unordered_set>
#include <functional>
#include "CollisionLayers.h"

class Collider2D : public Transform2D {
public:
    enum class ShapeType { Circle, Rectangle };

    ShapeType shapeType;
    int layer = 0;
    int mask = 0xFFFF;
    bool isTrigger = false;

    // Collision callbacks
    std::function<void(Collider2D*)> onCollisionEnter;
    std::function<void(Collider2D*)> onCollisionStay;
    std::function<void(Collider2D*)> onCollisionExit;

private:
    std::unordered_set<Collider2D*> currentCollisions;
    std::unordered_set<Collider2D*> collisionsThisFrame;

public:
    Collider2D(ShapeType shape) : shapeType(shape) {}

    // Call when a collision is detected
    void handleCollision(Collider2D* other) {
        collisionsThisFrame.insert(other);

        if (currentCollisions.find(other) == currentCollisions.end()) {
            // First frame of collision
            currentCollisions.insert(other);
            if (onCollisionEnter) onCollisionEnter(other);
        }
        else {
            // Continuing collision
            if (onCollisionStay) onCollisionStay(other);
        }
    }

    // Call once per frame after all collisions are processed
    void finalizeCollisions() {
        for (auto it = currentCollisions.begin(); it != currentCollisions.end();) {
            if (collisionsThisFrame.find(*it) == collisionsThisFrame.end()) {
                // Collision ended
                if (onCollisionExit) onCollisionExit(*it);
                it = currentCollisions.erase(it);
            }
            else {
                ++it;
            }
        }
        collisionsThisFrame.clear();
    }

    bool isCollidingWith(Collider2D* other) const {
        return currentCollisions.find(other) != currentCollisions.end();
    }
};
