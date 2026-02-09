#pragma once
#include <GL/glew.h>
#include <glm/glm.hpp>
#include <memory>
#include <vector>
#include <algorithm>

#include "GamepadInput.h"
#include "Actor3D.h"
#include "Model3D.h"
#include "Services.h"
#include "RenderSystem.h"

#include "stdio.h"


class GamepadObject : public Actor3D {

public:
	std::shared_ptr<Model3D> gamepad;
	std::weak_ptr<PlayerInput> input;
	
	float stickLimit = 1.0f;
	float triggerLimit = 0.8f;
	float buttonLimit = 5.0f;
	
	GamepadObject(std::weak_ptr<PlayerInput> player) {
		input = player;
	}

	void init() {
		gamepad = std::make_shared<Model3D>("gamepad");
		addChild(gamepad);

		Services::renderSystem->submit(gamepad);
	}


	void update(double dt) override {
		
		updateFromInput();
	}

private:
	void updateFromInput() {
		auto in = input.lock();
		if (in && in->enabled) {
			auto joystick = in->getPosition(Action::LeftStickX, Action::LeftStickY);
			
			float angle = length(joystick) * stickLimit;
			auto axis = glm::cross(vec3(joystick.x, joystick.y, 0), vec3(0, 0, 1));
			auto rotation = glm::quat(vec3(joystick.x, joystick.y, 0) * stickLimit);


			gamepad->applyTransform(glm::toMat4(rotation), "Stick_L");
			//gamepad->applyTransform(glm::toMat4(rotation), "Stick_L3");
		}

	}
};