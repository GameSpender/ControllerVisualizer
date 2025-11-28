#pragma once
#include <vector>
#include <algorithm>
#include <glm/glm.hpp>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/transform2.hpp>  // for glm::translate(), rotate(), scale()
#include <glm/gtx/matrix_operation.hpp>


using namespace glm;

class Transform2D {
public:
    // Local transform data
    vec2 position = vec2(0.0f);
    float rotation = 0.0f;        // radians
    vec2 scale = vec2(1.0f);

    // Hierarchy
    Transform2D* parent = nullptr;
    std::vector<Transform2D*> children;

private:
    bool dirty = true;
    mat3 cachedLocalMatrix = mat3(1.0f);
    mat3 cachedWorldMatrix = mat3(1.0f);

public:
    Transform2D() {}

    // ---------------- Hierarchy ----------------

    void setParent(Transform2D* newParent) {
        if (parent == newParent) return;

        // remove from old parent
        if (parent) {
            auto& siblings = parent->children;
            siblings.erase(std::remove(siblings.begin(), siblings.end(), this), siblings.end());
        }

        parent = newParent;

        if (parent)
            parent->children.push_back(this);

        markDirty();
    }

    void addChild(Transform2D* child) {
        if (std::find(children.begin(), children.end(), child) != children.end())
            return;
        children.push_back(child);
        child->setParent(this);
    }

    // ---------------- Dirty Propagation ----------------

    void markDirty() {
        dirty = true;
        for (auto* c : children)
            c->markDirty();
    }

    // ---------------- Matrix Builders ----------------

    mat3 calcLocalMatrix() const {
        // 2D transform = T * R * S
        mat3 T = mat3(1.0f);
        T[2] = vec3(position, 1.0f);

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
            cachedWorldMatrix = parent
                ? parent->getWorldMatrix() * cachedLocalMatrix
                : cachedLocalMatrix;

            dirty = false;
        }
        return cachedWorldMatrix;
    }

    const mat3& getParentWorldMatrix() {
        if (parent)
            return parent->getWorldMatrix();
        else
            return mat3(1.0f);
	}

    // ---------------- World-space helpers ----------------

    vec2 getWorldPosition() {
        const mat3& m = getWorldMatrix();
        return vec2(m[2].x, m[2].y);
    }

    float getWorldRotation() {
        const mat3& m = getWorldMatrix();
        return atan2(m[1][0], m[0][0]);
    }

    vec2 getWorldScale() {
        const mat3& m = getWorldMatrix();
        float sx = length(vec2(m[0][0], m[1][0]));
        float sy = length(vec2(m[0][1], m[1][1]));
        return vec2(sx, sy);
    }

	// ---------------- Virtual ----------------
    virtual void Update(double dt) {}
    virtual void Draw(SpriteRenderer&) {}
};
