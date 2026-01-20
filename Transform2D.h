#pragma once
#include <vector>
#include <algorithm>
#include <memory>
#include <glm/glm.hpp>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/transform2.hpp>  // for glm::translate(), rotate(), scale()
#include <glm/gtx/matrix_operation.hpp>

#include "SpriteRenderer.h"

using namespace glm;

class Transform2D : public std::enable_shared_from_this<Transform2D> {
public:
    // Local transform data
    vec2 position = vec2(0.0f);
    float rotation = 0.0f;        // radians
    vec2 scale = vec2(1.0f);

    // Hierarchy
    std::weak_ptr<Transform2D> parent;
    std::vector<std::shared_ptr<Transform2D>> children;

private:
    bool dirty = true;
    mat3 cachedLocalMatrix = mat3(1.0f);
    mat3 cachedWorldMatrix = mat3(1.0f);

public:
    Transform2D() {}

    // ---------------- Hierarchy ----------------

    void setParent(std::shared_ptr<Transform2D> newParent) {
        auto currentParent = parent.lock();
        if (currentParent == newParent) return;

        // remove from old parent
        if (currentParent) {
            auto& siblings = currentParent->children;
            siblings.erase(std::remove(siblings.begin(), siblings.end(), shared_from_this()), siblings.end());
        }

        parent = newParent;

        if (newParent)
            newParent->children.push_back(shared_from_this());

        markDirty();
    }

    void addChild(std::shared_ptr<Transform2D> child) {
        if (std::find(children.begin(), children.end(), child) != children.end())
            return;
        children.push_back(child);
        child->setParent(shared_from_this());
    }

    void removeChild(const std::shared_ptr<Transform2D>& child) {
        auto it = std::find(children.begin(), children.end(), child);
        if (it != children.end()) {
            (*it)->parent.reset();
            children.erase(it);
        }
    }



    // ---------------- Dirty Propagation ----------------

    void markDirty() {
        dirty = true;
        for (auto& c : children)
            c->markDirty();
    }

    // ---------------- Matrix Builders ----------------

    mat3 calcLocalMatrix() {
        // 2D transform = T * R * S
        mat3 T = mat3(1.0f);
        T[2] = vec3(position, 1.0f);

        rotation = std::fmod(rotation, glm::two_pi<float>());
        if (rotation < 0.0f)
            rotation += glm::two_pi<float>();

        mat3 R = mat3(
            cos(rotation), -sin(rotation), 0,
            sin(rotation), cos(rotation), 0,
            0, 0, 1
        );

        mat3 S = mat3(
            scale.x, 0, 0,
            0, scale.y, 0,
            0, 0, 1
        );

        return T * R * S;
    }

    // ---------------- Getters ----------------

    const mat3& getLocalMatrix() {
        if (dirty)
            cachedLocalMatrix = calcLocalMatrix();
        return cachedLocalMatrix;
    }

    const mat3& getWorldMatrix() {
        if (dirty) {
            cachedLocalMatrix = calcLocalMatrix();

            if (auto p = parent.lock()) {
                cachedWorldMatrix = p->getWorldMatrix() * cachedLocalMatrix;
            }
            else {
                cachedWorldMatrix = cachedLocalMatrix;
            }

            dirty = false;
        }
        return cachedWorldMatrix;
    }

    const mat3& getParentWorldMatrix() {
        if (auto p = parent.lock()) { 
            return p->getWorldMatrix();
        }
        else {
            static mat3 identity(1.0f);
            return identity;
        }
	}

    // ---------------- World-space helpers ----------------

    glm::vec2 getWorldPosition() {
        const mat3& m = getWorldMatrix();
        return vec2(m[2].x, m[2].y);
    }

    float getWorldRotation() {
        const mat3& m = getWorldMatrix();
        return atan2(m[1][0], m[0][0]);
    }

    glm::vec2 getWorldScale() {
        const mat3& m = getWorldMatrix();
        float sx = length(vec2(m[0][0], m[1][0]));
        float sy = length(vec2(m[0][1], m[1][1]));
        return vec2(sx, sy);
    }

    void setRotation(vec2 dir) {
        if (length(dir) == 0.0f) return; 
        rotation = atan2(dir.y, -dir.x) + radians(90.0f);
    }


	// ---------------- Virtual ----------------
    virtual void update(double dt) {}
};
