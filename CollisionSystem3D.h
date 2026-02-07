#pragma once
#include <vector>
#include <memory>
#include <unordered_set>
#include <functional>
#include "Collider3D.h"
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtx/norm.hpp"

using namespace glm;

// --- OBB structure for 3D boxes ---
struct OBB3D {
    vec3 center;
    vec3 axes[3];     // normalized world axes
    vec3 extents;     // half-width/height/depth (from transform scale)
};

inline OBB3D getOBB(Collider3D& c) {
    const mat4& M = c.getWorldMatrix();

    OBB3D o;
    o.center = vec3(M[3]); // world translation

    // extract world axes from rotation columns
    o.axes[0] = normalize(vec3(M[0])); // right
    o.axes[1] = normalize(vec3(M[1])); // up
    o.axes[2] = normalize(vec3(M[2])); // forward

    vec3 scale = c.getWorldScale();
    o.extents = scale * 0.5f;

    return o;
}

// --- Projected overlap test on axis ---
inline bool overlapOnAxis(const OBB3D& A, const OBB3D& B, const vec3& axis) {
    if (length2(axis) < 1e-6f) return true; // ignore degenerate axes

    float projA =
        fabs(dot(A.axes[0], axis)) * A.extents.x +
        fabs(dot(A.axes[1], axis)) * A.extents.y +
        fabs(dot(A.axes[2], axis)) * A.extents.z;

    float projB =
        fabs(dot(B.axes[0], axis)) * B.extents.x +
        fabs(dot(B.axes[1], axis)) * B.extents.y +
        fabs(dot(B.axes[2], axis)) * B.extents.z;

    float distance = fabs(dot(B.center - A.center, axis));
    return distance <= projA + projB;
}

// --- Box vs Box collision ---
inline bool obbVsObb(Collider3D& A_c, Collider3D& B_c) {
    OBB3D A = getOBB(A_c);
    OBB3D B = getOBB(B_c);

    vec3 axes[15];
    int k = 0;

    // face axes
    for (int i = 0; i < 3; i++) axes[k++] = A.axes[i];
    for (int i = 0; i < 3; i++) axes[k++] = B.axes[i];

    // cross-product axes
    for (int i = 0; i < 3; i++)
        for (int j = 0; j < 3; j++)
            axes[k++] = cross(A.axes[i], B.axes[j]);

    for (int i = 0; i < 15; i++)
        if (!overlapOnAxis(A, B, axes[i]))
            return false;

    return true;
}

// --- Sphere vs Sphere ---
inline bool sphereVsSphere(Collider3D& A, Collider3D& B) {
    vec3 d = A.getWorldPosition() - B.getWorldPosition();
    float rA = A.scale.x * A.getWorldScale().x;
    float rB = B.scale.x * B.getWorldScale().x;
    return dot(d, d) <= (rA + rB) * (rA + rB);
}

// --- Sphere vs OBB ---
inline bool sphereVsObb(Collider3D& S, Collider3D& B_c) {
    OBB3D B = getOBB(B_c);
    vec3 pc = S.getWorldPosition();
    float r = S.scale.x * S.getWorldScale().x;

    vec3 d = pc - B.center;

    // transform point into box local space
    vec3 closest(0.0f);
    for (int i = 0; i < 3; i++) {
        float dist = dot(d, B.axes[i]);
        dist = glm::clamp(dist, -B.extents[i], B.extents[i]);
        closest += B.axes[i] * dist;
    }

    vec3 delta = pc - (B.center + closest);
    return dot(delta, delta) <= r * r;
}

// --- Simple Cylinder vs Cylinder approximation (vertical Y) ---
inline bool cylinderVsCylinder(Collider3D& A, Collider3D& B) {
    vec3 d = A.getWorldPosition() - B.getWorldPosition();
    float rA = A.scale.x * A.getWorldScale().x;
    float rB = B.scale.x * B.getWorldScale().x;

    // horizontal distance
    vec2 deltaXZ(d.x, d.z);
    if (dot(deltaXZ, deltaXZ) > (rA + rB) * (rA + rB)) return false;

    // vertical overlap
    float yAmin = A.getWorldPosition().y;
    float yAmax = yAmin + A.scale.y * A.getWorldScale().y;
    float yBmin = B.getWorldPosition().y;
    float yBmax = yBmin + B.scale.y * B.getWorldScale().y;

    return (yAmin <= yBmax && yAmax >= yBmin);
}

// --- Main collision test ---
inline void testCollision(std::shared_ptr<Collider3D> A, std::shared_ptr<Collider3D> B) {
    if (!A || !B) return;

    if ((A->mask & B->layer) == 0 || (B->mask & A->layer) == 0)
        return;

    bool collision = false;

    if (A->shapeType == Collider3D::ShapeType3D::Box && B->shapeType == Collider3D::ShapeType3D::Box)
        collision = obbVsObb(*A, *B);
    else if (A->shapeType == Collider3D::ShapeType3D::Sphere && B->shapeType == Collider3D::ShapeType3D::Sphere)
        collision = sphereVsSphere(*A, *B);
    else if ((A->shapeType == Collider3D::ShapeType3D::Sphere && B->shapeType == Collider3D::ShapeType3D::Box) ||
        (A->shapeType == Collider3D::ShapeType3D::Box && B->shapeType == Collider3D::ShapeType3D::Sphere)) {
        if (A->shapeType != Collider3D::ShapeType3D::Sphere) std::swap(A, B);
        collision = sphereVsObb(*A, *B);
    }
    else if (A->shapeType == Collider3D::ShapeType3D::Cylinder && B->shapeType == Collider3D::ShapeType3D::Cylinder)
        collision = cylinderVsCylinder(*A, *B);

    if (collision) {
        A->handleCollision(B.get());
        B->handleCollision(A.get());
    }
}

// --- Collision system ---
class CollisionSystem3D {
public:
    std::vector<std::weak_ptr<Collider3D>> colliders;

    void addCollider(const std::shared_ptr<Collider3D>& c) {
        colliders.push_back(c);
    }

    void update() {
        // remove expired
        colliders.erase(
            std::remove_if(colliders.begin(), colliders.end(),
                [](const std::weak_ptr<Collider3D>& w) { return w.expired(); }),
            colliders.end()
        );

        // pairwise collisions
        for (int i = 0; i < colliders.size(); i++) {
            auto a = colliders[i].lock();
            if (!a) continue;
            for (int j = i + 1; j < colliders.size(); j++) {
                auto b = colliders[j].lock();
                if (!b) continue;
                testCollision(a, b);
            }
        }

        // finalize callbacks
        for (auto& w : colliders) {
            if (auto c = w.lock()) c->finalizeCollisions();
        }
    }
};
