#pragma once
#include "Actor3D.h"
#include "IControllable.h"
#include <glm/glm.hpp>
#include <glm/gtx/quaternion.hpp>

class UprightActor3D : public Actor3D, public IControllable {
public:
    float moveSpeed = 30.0f;
    float verticalSpeed = 50.0f;

    float aimSensitivity = 2.5f;     // how fast camera-style turning feels

private:
    glm::vec2 moveInput = glm::vec2(0.0f);

	glm::vec2 aimVelocity = glm::vec2(0.0f); // x = yaw velocity, y = pitch velocity


    bool moveUp = false;
    bool moveDown = false;

public:
    void update(double dt) override {
        float fdt = static_cast<float>(dt);

        // ----------------------------------
        // 1. Apply aim input to yaw/pitch
        // ----------------------------------

        if (glm::length(aimVelocity) > 0.00001f) {

            float deltaYaw = aimVelocity.x * fdt;
            float deltaPitch = - aimVelocity.y * fdt;

            // --- YAW (around world Y axis) ---
            if (fabs(deltaYaw) > 0.00001f) {
                glm::quat yawRot = glm::angleAxis(-deltaYaw, glm::vec3(0, 1, 0));
                rotation = glm::normalize(yawRot * rotation);
            }

            // --- PITCH (around local right axis) ---
            if (fabs(deltaPitch) > 0.00001f) {

                glm::quat pitchRot = glm::angleAxis(deltaPitch, right());
                glm::quat newRot = glm::normalize(pitchRot * rotation);

                // --- Clamp vertical angle ---
                glm::vec3 forward = newRot * glm::vec3(0, 0, 1);
                float verticalDot = glm::dot(forward, glm::vec3(0, 1, 0));

                // verticalDot = sin(pitch)
                // clamp to ±90 degrees → dot must stay between -1 and 1
                float limit = 0.999f; // slightly below 1 to avoid instability

                if (verticalDot < limit && verticalDot > -limit) {
                    rotation = newRot;
                }
            }

            // Remove roll (force upright)
            glm::vec3 forward = rotation * glm::vec3(0, 0, 1);
            forward.y = glm::clamp(forward.y, -0.999f, 0.999f);
            forward = glm::normalize(forward);

            glm::vec3 right = glm::normalize(glm::cross(glm::vec3(0, 1, 0), forward));
            glm::vec3 up = glm::cross(forward, right);

            glm::mat3 m(right, up, forward);
            rotation = glm::normalize(glm::quat_cast(glm::mat4(m)));

            markDirty();
        }

        // ----------------------------------
        // 2. Horizontal movement (XZ plane)
        // ----------------------------------

        glm::vec3 move =
            forward() * -moveInput.y +
            right() * moveInput.x;

        if (glm::length(move) > 0.001f) {
            move = glm::normalize(move);
            position += move * moveSpeed * fdt;
            markDirty();
        }

        // ----------------------------------
        // 3. Vertical movement
        // ----------------------------------

        float vertical = 0.0f;
        if (moveUp) vertical += 1.0f;
        if (moveDown) vertical -= 1.0f;

        if (vertical != 0.0f) {
            position.y += vertical * verticalSpeed * fdt;
            markDirty();
        }

        Actor3D::update(dt);
    }

    // ----------------------------
    // IControllable
    // ----------------------------

    void setMoveDirection(const glm::vec2 dir) override {
        moveInput = (glm::length(dir) > 1.0f) ? glm::normalize(dir) : dir;
    }

    // Now aimDirection is treated as yaw delta input
    void setAimDirection(const glm::vec2 dir) override {
        // dir.x = mouse X or right stick X
		aimVelocity = dir * aimSensitivity;
    }

    void useAbility(int index, bool pressed) override {
        if (index == 0) moveUp = pressed;
        if (index == 1) moveDown = pressed;
    }

    void dash(bool pressed) override {
        if (!pressed) return;

        glm::vec3 forward = rotation * glm::vec3(0, 0, 1);
        position += forward * 5.0f;
        markDirty();
    }
};
