#pragma once
#include "IControllable.h"
#include "InputSystem.h"
#include "Actor3D.h"
#include "PhysicalActor3D.h"
#include <glm/glm.hpp>

class PlayerController {
    IControllable* pawn = nullptr;
    Actor3D* actorPawn = nullptr;
    PhysicalActor3D* physPawn = nullptr;

    PlayerInput* input = nullptr;

public:
    PlayerController(PlayerInput* pi) : input(pi) {}

    void possess(IControllable* target) {
        pawn = target;
        actorPawn = dynamic_cast<Actor3D*>(pawn);
        physPawn = dynamic_cast<PhysicalActor3D*>(pawn);
    }

    void unpossess() {
        pawn = nullptr;
        actorPawn = nullptr;
        physPawn = nullptr;
    }

    void update(double dt) {
        if (!pawn || !input) return;

        // Movement
        pawn->setMoveDirection(input->getPosition(Action::MoveHorizontal, Action::MoveVertical));

        // Aim
        pawn->setAimDirection(input->getPosition(Action::AimHorizontal, Action::AimVertical));

        if (input->bindings.find(Action::MousePositionHorizontal) != input->bindings.end() ||
            input->bindings.find(Action::MousePositionVertical) != input->bindings.end()) {
            // If mouse position is bound, use it for aiming
            glm::vec2 pawnPos = pawnPosition();
			pawn->setAimDirection(input->getPosition(Action::MousePositionHorizontal, Action::MousePositionVertical) - pawnPos);
		}

		pawn->useAbility(0, input->isDown(Action::PrimaryAbility));
		pawn->useAbility(1, input->isDown(Action::SecondaryAbility));
		pawn->useAbility(2, input->isDown(Action::TertiaryAbility));
		pawn->useAbility(3, input->isDown(Action::QuaternaryAbility));

        // Dash
        pawn->dash(input->isDown(Action::Dash));
    }

    glm::vec3 pawnPosition() const {
        if (actorPawn)
            return actorPawn->getWorldPosition();
        return glm::vec3(0.0f);
    }

    glm::vec3 pawnVelocity() const {
        if (physPawn)
            return physPawn->getVelocity();
        return glm::vec3(0.0f);
    }
};
