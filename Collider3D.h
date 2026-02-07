#pragma once
#include "Transform3D.h"
#include <unordered_set>
#include <functional>
#include "CollisionLayers.h"

class Collider3D : public Transform3D {
public:
    enum class ShapeType3D { Sphere, Box, Cylinder };

    ShapeType3D shapeType;
    int layer = 0;
    int mask = 0xFFFF;
    bool isTrigger = false;

    // Collision callbacks
    std::function<void(Collider3D*)> onCollisionEnter;
    std::function<void(Collider3D*)> onCollisionStay;
    std::function<void(Collider3D*)> onCollisionExit;

private:
    std::unordered_set<Collider3D*> currentCollisions;
    std::unordered_set<Collider3D*> collisionsThisFrame;

public:
    Collider3D(ShapeType3D shape) : shapeType(shape) {}

    // Call when a collision is detected
    void handleCollision(Collider3D* other) {
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

    bool isCollidingWith(Collider3D* other) const {
        return currentCollisions.find(other) != currentCollisions.end();
    }
};
