#pragma once
#include "IControllable.h"
#include "InputSystem.h"
#include <glm/glm.hpp>


class PlayerController {
    IControllable* pawn = nullptr;
    PlayerInput* input = nullptr;

public:
    PlayerController(PlayerInput* pi) : input(pi) {}

    void possess(IControllable* target) {
        pawn = target;
    }
    
    void unpossess() {
        pawn = nullptr;
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
            glm::vec2 pawnPosition = pawn->getPostion();
			pawn->setAimDirection(input->getPosition(Action::MousePositionHorizontal, Action::MousePositionVertical) - pawnPosition);
		}

		pawn->useAbility(0, input->isDown(Action::PrimaryAbility));
		pawn->useAbility(1, input->isDown(Action::SecondaryAbility));
		pawn->useAbility(2, input->isDown(Action::TertiaryAbility));
		pawn->useAbility(3, input->isDown(Action::QuaternaryAbility));

        // Dash
        pawn->dash(input->isDown(Action::Dash));
    }
};
