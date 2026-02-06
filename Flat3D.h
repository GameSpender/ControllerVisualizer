#pragma once
#include "Transform3D.h"
#include "Transform2D.h"

class Flat3D : public Transform3D {
public:
    std::shared_ptr<Transform2D> flat2DChild; // optional 2D hierarchy

    Flat3D() : flat2DChild(nullptr) {}

    void setFlat2DChild(std::shared_ptr<Transform2D> child) {
        flat2DChild = child;
    }

    // ---------------- Flattened 2D world matrix in 3D space ----------------
    mat4 getFlattened2DWorldMatrix() {
        if (!flat2DChild) return getWorldMatrix(); // fallback to just 3D world

        const mat3& m2d = flat2DChild->getWorldMatrix();

        mat4 m2d4(1.0f);

        // 2D rotation + scale (upper-left 2x2)
        m2d4[0][0] = m2d[0][0]; m2d4[0][1] = m2d[0][1];
        m2d4[1][0] = m2d[1][0]; m2d4[1][1] = m2d[1][1];

        // 2D translation
        m2d4[3][0] = m2d[2][0];
        m2d4[3][1] = m2d[2][1];

        // Merge with this Transform3D's world matrix
        return getWorldMatrix() * m2d4;
    }
};
