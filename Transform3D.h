#pragma once
#include <vector>
#include <algorithm>
#include <memory>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>  // translate, rotate, scale
#include <glm/gtx/quaternion.hpp>        // for quaternion rotations
#include <glm/gtx/matrix_decompose.hpp> // for decompose

using namespace glm;

class Transform3D : public std::enable_shared_from_this<Transform3D> {
public:
    std::string name = "Transform3D";
    // Local transform
    vec3 position = vec3(0.0f);
    quat rotation = glm::angleAxis(0.0f, vec3(0, 1, 0)); // no rotation needed
    vec3 scale = vec3(1.0f);

	bool deleted = false; // for cleanup

    // Hierarchy
    std::weak_ptr<Transform3D> parent;
    std::vector<std::shared_ptr<Transform3D>> children;


private:
    bool dirty = true;
    mat4 cachedLocalMatrix = mat4(1.0f);
    mat4 cachedWorldMatrix = mat4(1.0f);

public:
    Transform3D() {}

    // ---------------- Hierarchy ----------------
    void setParent(std::shared_ptr<Transform3D> newParent) {
        auto currentParent = parent.lock();
        if (currentParent == newParent) return;

        if (currentParent) {
            auto& siblings = currentParent->children;
            siblings.erase(std::remove(siblings.begin(), siblings.end(), shared_from_this()), siblings.end());
        }

        parent = newParent;
        if (newParent) newParent->children.push_back(shared_from_this());
        markDirty();
    }

    inline void reparentKeepWorld(const std::shared_ptr<Transform3D>& child,
        const std::shared_ptr<Transform3D>& newParent)
    {
        if (!child) return;

        // Current world transform
        glm::mat4 world = child->getWorldMatrix();

        // Set new parent first (temporarily resets dirty hierarchy)
        child->setParent(newParent);

        // Compute new local transform relative to new parent
        glm::mat4 parentWorldInv(1.0f);
        if (newParent) parentWorldInv = glm::inverse(newParent->getWorldMatrix());

        glm::mat4 local = parentWorldInv * world;

        // Decompose local matrix into position, rotation, scale
        glm::vec3 skew;
        glm::vec4 perspective;
        glm::decompose(local, child->scale, child->rotation, child->position, skew, perspective);

        child->markDirty();
    }


    void addChild(std::shared_ptr<Transform3D> child) {
        if (!child) return;
        child->setParent(shared_from_this());
    }


    void removeChild(const std::shared_ptr<Transform3D>& child) {
        auto it = std::find(children.begin(), children.end(), child);
        if (it != children.end()) {
            (*it)->parent.reset();
            children.erase(it);
        }
    }


    void markDeleted() {
        deleted = true;
        // Propagate deleted status to all children
        for (auto& child : children) {
            if (child && !child->deleted)
                child->markDeleted();
        }
    }

    // ---------------- Dirty Propagation ----------------
    void markDirty() {
        dirty = true;
        for (auto& c : children) c->markDirty();
    }

    // ---------------- Matrix Builders ----------------
    mat4 calcLocalMatrix() const {
        mat4 T = glm::translate(mat4(1.0f), position);
        mat4 R = glm::toMat4(rotation);
        mat4 S = glm::scale(mat4(1.0f), scale);
        return T * R * S;
    }

    // ---------------- Getters ----------------
    const mat4& getLocalMatrix() {
        if (dirty) cachedLocalMatrix = calcLocalMatrix();
        return cachedLocalMatrix;
    }

    const mat4& getWorldMatrix() {
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

    const mat4& getParentWorldMatrix() {
        if (auto p = parent.lock()) return p->getWorldMatrix();
        static mat4 identity(1.0f);
        return identity;
    }

    // ---------------- World-space helpers ----------------
    vec3 getWorldPosition() {
        const mat4& m = getWorldMatrix();
        return vec3(m[3]); // translation part
    }

    quat getWorldRotation() {
        if (auto p = parent.lock()) return p->getWorldRotation() * rotation;
        return rotation;
    }

    vec3 getWorldScale() {
        const mat4& m = getWorldMatrix();
        return vec3(
            length(vec3(m[0])),
            length(vec3(m[1])),
            length(vec3(m[2]))
        );
    }

    void lookAt(const vec3& target, const vec3& up = vec3(0, 1, 0)) {
        mat4 mat = glm::lookAt(position, target, up);
        rotation = glm::quat_cast(glm::inverse(mat));
        markDirty();
    }

    // ---------------- Virtual ----------------
    virtual void update(double dt) {}
};
