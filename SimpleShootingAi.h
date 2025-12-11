#pragma once
#include "IControllable.h"
#include <glm/glm.hpp>

class SimpleShootingAi {
    IControllable* pawn = nullptr;

    Actor2D* actorPawn = nullptr;
    PhysicalActor2D* physPawn = nullptr;

    glm::vec2 targetPosition = glm::vec2(0.0f);
    glm::vec2 targetVelocity = glm::vec2(0.0f);

    float preferredDistance = 500.0f; // desired distance from target
    float maxMoveSpeed = 1.0f;        // normalized 0..1 input for pawn movement

public:
    SimpleShootingAi() = default;

    void possess(IControllable* target) {
        pawn = target;
        actorPawn = dynamic_cast<Actor2D*>(pawn);
        physPawn = dynamic_cast<PhysicalActor2D*>(pawn);
    }

    void unpossess() {
        pawn = nullptr;
        actorPawn = nullptr;
        physPawn = nullptr;
    }

    // Update the target info (from Director)
    void setTarget(const glm::vec2& pos, const glm::vec2& vel) {
        targetPosition = pos;
        targetVelocity = vel;
    }

    void update(double dt) {
        if (!pawn) return;
        glm::vec2 pawnPos = pawnPosition();

        glm::vec2 toTarget = targetPosition - pawnPos;
        float distance = glm::length(toTarget);
        glm::vec2 direction = (distance > 0.001f) ? glm::normalize(toTarget) : glm::vec2(0.0f);

        // -----------------------------
        // 1. Movement: maintain distance
        // -----------------------------
        glm::vec2 moveInput(0.0f);

        if (distance > preferredDistance + 20.0f) {
            moveInput = direction; // move toward
        }
        else if (distance < preferredDistance - 20.0f) {
            moveInput = -direction; // move away
        }

        // Add perpendicular strafing safely
        glm::vec2 strafe(-direction.y, direction.x);
        if (glm::length(strafe) > 0.001f) // safe to normalize
            strafe = glm::normalize(strafe);

        moveInput += strafe * 0.0f;

        // Clamp magnitude to 1
        float moveLen = glm::length(moveInput);
        if (moveLen > 1.0f)
            moveInput /= moveLen;

        pawn->setMoveDirection(moveInput * maxMoveSpeed);

        // -----------------------------
        // 2. Aim directly at target
        // -----------------------------
        pawn->setAimDirection(direction);

        // -----------------------------
        // 4. Evade if too close
        // -----------------------------
        bool tooClose = distance < preferredDistance * 0.7f;
        if (tooClose)
            pawn->setAimDirection(glm::vec2(0.0f)); // stop aiming when running away

        // -----------------------------
        // 3. Abilities: fire if within distance
        // -----------------------------
        bool inRange = distance <= preferredDistance * 3.0f;
        if (tooClose) inRange = false; // don't fire when too close
        pawn->useAbility(0, inRange); // primary fire
        // Additional abilities can be handled similarly
    }

    glm::vec2 pawnPosition() const {
        if (actorPawn)
            return actorPawn->getWorldPosition();
        return glm::vec2(0.0f);
    }

    glm::vec2 pawnVelocity() const {
        if (physPawn)
            return physPawn->getVelocity();
        return glm::vec2(0.0f);
    }


};
