#pragma once
#include <vector>
#include "Collider.h"





struct OBB {
    vec2 center;
    vec2 axes[2];     // normalized world axes
    float extents[2]; // half width/height (from transform scale)
};

inline OBB getOBB(Collider2D& c) {
    const mat3& M = c.getWorldMatrix();

    OBB o;

    // World position
    o.center = vec2(M[2].x, M[2].y);

    // Extract orientation from matrix columns
    vec2 axisX = vec2(M[0].x, M[1].x);
    vec2 axisY = vec2(M[0].y, M[1].y);

    float lenX = glm::length(axisX);
    float lenY = glm::length(axisY);

    o.axes[0] = axisX / lenX;  // normalized X axis
    o.axes[1] = axisY / lenY;  // normalized Y axis

    // Use world-scale directly as extents
    vec2 worldScale = c.getWorldScale();
    o.extents[0] = worldScale.x * 0.5f;
    o.extents[1] = worldScale.y * 0.5f;

    return o;
}

inline bool overlapOnAxis(const OBB& A, const OBB& B, const vec2& axis)
{
    float projA =
        fabs(dot(A.axes[0], axis)) * A.extents[0] +
        fabs(dot(A.axes[1], axis)) * A.extents[1];

    float projB =
        fabs(dot(B.axes[0], axis)) * B.extents[0] +
        fabs(dot(B.axes[1], axis)) * B.extents[1];

    float distance = fabs(dot(B.center - A.center, axis));

    return distance <= projA + projB;
}

bool obbVsObb(Collider2D& A_c, Collider2D& B_c)
{
    OBB A = getOBB(A_c);
    OBB B = getOBB(B_c);

    vec2 axes[] = { A.axes[0], A.axes[1], B.axes[0], B.axes[1] };

    for (const auto& axis : axes)
        if (!overlapOnAxis(A, B, axis))
            return false;

    return true;
}

bool circleVsObb(Collider2D& C, Collider2D& B_c)
{
    OBB B = getOBB(B_c);

    vec2 pc = C.getWorldPosition();
    float rc = C.scale.x * C.getWorldScale().x;

    vec2 d = pc - B.center;

    float localX = dot(d, B.axes[0]);
    float localY = dot(d, B.axes[1]);

    float clampedX = glm::clamp(localX, -B.extents[0], B.extents[0]);
    float clampedY = glm::clamp(localY, -B.extents[1], B.extents[1]);

    vec2 closest =
        B.center +
        B.axes[0] * clampedX +
        B.axes[1] * clampedY;

    vec2 delta = pc - closest;
    float dist2 = glm::dot(delta, delta);
    return dist2 <= rc * rc;
}

inline void testCollision(std::shared_ptr<Collider2D> A, std::shared_ptr<Collider2D> B)
{
    if (!A || !B) return;

    if ((A->mask & B->layer) == 0 || (B->mask & A->layer) == 0)
        return;

    bool collision = false;

    // Determine collision type based on shape
    if (A->shapeType == Collider2D::ShapeType::Rectangle && B->shapeType == Collider2D::ShapeType::Rectangle) {
        // OBB vs OBB
        collision = obbVsObb(*A, *B);
    }
    else if (A->shapeType == Collider2D::ShapeType::Circle && B->shapeType == Collider2D::ShapeType::Circle) {
        // Circle vs Circle
        vec2 posA = A->getWorldPosition();
        vec2 posB = B->getWorldPosition();
        float rA = A->scale.x * A->getWorldScale().x;
        float rB = B->scale.x * B->getWorldScale().x;
        vec2 delta = posB - posA;
        float dist2 = dot(delta, delta);
        float radiusSum = rA + rB;
        collision = dist2 <= radiusSum * radiusSum;
    }
    else {
        // Circle vs OBB: make sure circle is first
        if (A->shapeType != Collider2D::ShapeType::Circle) std::swap(A, B);
        collision = circleVsObb(*A, *B);
    }

    if (collision) {
        // Optional: call collision callbacks if you have them
        A->handleCollision(B.get());
        B->handleCollision(A.get());
    }
}

class CollisionSystem {
public:
    std::vector<std::weak_ptr<Collider2D>> colliders;

    void addCollider(const std::shared_ptr<Collider2D>& c) {
        colliders.push_back(c);
    }

    void update() {
        colliders.erase(
            std::remove_if(colliders.begin(), colliders.end(),
                [](const std::weak_ptr<Collider2D>& w) {
                    return w.expired();
                }
            ),
            colliders.end()
        );

        // pairwise collision test
        for (int i = 0; i < colliders.size(); i++) {
            auto a = colliders[i].lock();
            if (!a) continue;

            for (int j = i + 1; j < colliders.size(); j++) {
                auto b = colliders[j].lock();
                if (!b) continue;

                testCollision(a, b);
            }
        }

        for (int i = 0; i < colliders.size(); i++) {
            auto a = colliders[i].lock();
            a->finalizeCollisions();
        }
    }
};