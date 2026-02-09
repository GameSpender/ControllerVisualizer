#pragma once

#include "InputSystem.h"

void bindKeyboardAndMouse(InputDevice keyboard, InputDevice mouse, PlayerInput& playerInput, int screenHeight) {
    playerInput.devices.push_back(keyboard);
    playerInput.devices.push_back(mouse);
    // ---------------- Keyboard ----------------
    InputBinding moveRight{ .device = keyboard, .code = GLFW_KEY_D, .scale = 1.0f };
    InputBinding moveLeft{ .device = keyboard, .code = GLFW_KEY_A, .scale = -1.0f };
    InputBinding moveUp{ .device = keyboard, .code = GLFW_KEY_W, .scale = -1.0f };
    InputBinding moveDown{ .device = keyboard, .code = GLFW_KEY_S, .scale = 1.0f };

    playerInput.bindings[Action::MoveHorizontal].push_back(moveRight);
    playerInput.bindings[Action::MoveHorizontal].push_back(moveLeft);

    playerInput.bindings[Action::MoveVertical].push_back(moveUp);
    playerInput.bindings[Action::MoveVertical].push_back(moveDown);

    // ---------------- Mouse axes ----------------
    InputBinding mouseX{ .device = mouse, .axisCode = 0, .scale = 1.0f };
    InputBinding mouseY{ .device = mouse, .axisCode = 1, .scale = -1.0f, .offset = static_cast<float>(screenHeight) };

    playerInput.bindings[Action::MousePositionHorizontal].push_back(mouseX);
    playerInput.bindings[Action::MousePositionVertical].push_back(mouseY);

    // ---------------- Mouse buttons ----------------
    InputBinding mouseRight{ .device = mouse, .code = GLFW_MOUSE_BUTTON_2 };
    InputBinding mouseLeft{ .device = mouse, .code = GLFW_MOUSE_BUTTON_1 };

    playerInput.bindings[Action::SecondaryAbility].push_back(mouseRight);
    playerInput.bindings[Action::PrimaryAbility].push_back(mouseLeft);
}

void bindGamepad(InputDevice gamepad, PlayerInput& playerInput) {
    playerInput.devices.push_back(gamepad);
    // Left stick: movement
    InputBinding leftStickX{ .device = gamepad, .axisCode = GLFW_GAMEPAD_AXIS_LEFT_X, .scale = 1.0f, .deadzone = 0.05f };
    InputBinding leftStickY{ .device = gamepad, .axisCode = GLFW_GAMEPAD_AXIS_LEFT_Y, .scale = 1.0f, .deadzone = 0.05f };

    playerInput.bindings[Action::MoveHorizontal].push_back(leftStickX);
    playerInput.bindings[Action::MoveVertical].push_back(leftStickY);

    // Right stick: aiming
    InputBinding rightStickX{ .device = gamepad, .axisCode = GLFW_GAMEPAD_AXIS_RIGHT_X, .scale = 1.0f, .deadzone = 0.05f };
    InputBinding rightStickY{ .device = gamepad, .axisCode = GLFW_GAMEPAD_AXIS_RIGHT_Y, .scale = 1.0f, .deadzone = 0.05f };

    playerInput.bindings[Action::AimHorizontal].push_back(rightStickX);
    playerInput.bindings[Action::AimVertical].push_back(rightStickY);

    // Right bumper: shoot
    InputBinding rightBumper{ .device = gamepad, .code = GLFW_GAMEPAD_BUTTON_RIGHT_BUMPER };
	InputBinding leftBumper{ .device = gamepad, .code = GLFW_GAMEPAD_BUTTON_LEFT_BUMPER };
    playerInput.bindings[Action::PrimaryAbility].push_back(rightBumper);
	playerInput.bindings[Action::SecondaryAbility].push_back(leftBumper);
}
