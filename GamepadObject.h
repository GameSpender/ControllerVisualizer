#pragma once
#include "Transform2D.h"

#include <GL/glew.h>
#include <glm/glm.hpp>
#include <memory>
#include <vector>
#include <algorithm>

#include "AnalogStickObject.h"
#include "ButtonObject.h"

#include "GamepadInput.h"
#include "Actor.h"

using namespace glm;

struct GamepadTextures{
	unsigned int gamepadBody;
	unsigned int stickHead;
	unsigned int stickHeadPressed;
	unsigned int buttonAIdle;
	unsigned int buttonAPressed;
	unsigned int buttonBIdle;
	unsigned int buttonBPressed;
	unsigned int buttonXIdle;
	unsigned int buttonXPressed;
	unsigned int buttonYIdle;
	unsigned int buttonYPressed;
	unsigned int dpadIdle;
	unsigned int dpadPressed;
	unsigned int bumper;
	unsigned int bumperPressed;
};;

class GamepadObject : public Actor2D {

	unsigned int gamepadBody;
public:
	std::shared_ptr<AnalogStickObject> leftStick;
	std::shared_ptr<AnalogStickObject> rightStick;

	std::shared_ptr<ButtonObject> buttonA;
	std::shared_ptr<ButtonObject> buttonB;
	std::shared_ptr<ButtonObject> buttonX;
	std::shared_ptr<ButtonObject> buttonY;

	std::shared_ptr<ButtonObject> dpadUp;
	std::shared_ptr<ButtonObject> dpadDown;
	std::shared_ptr<ButtonObject> dpadLeft;
	std::shared_ptr<ButtonObject> dpadRight;

	std::shared_ptr<ButtonObject> bumperLeft;
	std::shared_ptr<ButtonObject> bumperRight;

public:

	GamepadObject()
	{// Setup hierarchy
		spriteName = "gamepad_body";
		leftStick = std::make_shared<AnalogStickObject>();
		rightStick = std::make_shared<AnalogStickObject>();

		buttonA = std::make_shared<ButtonObject>("button_A", "button_A_pressed");
		buttonB = std::make_shared<ButtonObject>("button_B", "button_B_pressed");
		buttonX = std::make_shared<ButtonObject>("button_X", "button_X_pressed");
		buttonY = std::make_shared<ButtonObject>("button_Y", "button_Y_pressed");

		dpadUp = std::make_shared<ButtonObject>("dpad", "dpad_pressed");
		dpadDown = std::make_shared<ButtonObject>("dpad", "dpad_pressed");
		dpadLeft = std::make_shared<ButtonObject>("dpad", "dpad_pressed");
		dpadRight = std::make_shared<ButtonObject>("dpad", "dpad_pressed");

		bumperLeft = std::make_shared<ButtonObject>("bumper", "bumper_pressed");
		bumperRight = std::make_shared<ButtonObject>("bumper", "bumper_pressed");

		// Position components appropriately (example positions, adjust as needed)
		double tempscale = 1.0f / 100.0f;

		leftStick->setOrigin(vec2(-12.0f * tempscale, 0.0f * tempscale));
		leftStick->scale = vec2(10.0f * tempscale);
		leftStick->displacementFactor = vec2(0.4f);
		rightStick->setOrigin(vec2(12.0f * tempscale, 0.0f * tempscale));
		rightStick->scale = vec2(10.0f * tempscale);
		rightStick->displacementFactor = vec2(0.4f);

		// ABXY buttons

		buttonA->position = vec2(24.0f * tempscale, -8.0f * tempscale);
		buttonA->scale = vec2(6.0f * tempscale);

		buttonB->position = vec2(30.0f * tempscale, -14.0f * tempscale);
		buttonB->scale = vec2(6.0f * tempscale);

		buttonX->position = vec2(18.0f * tempscale, -14.0f * tempscale);
		buttonX->scale = vec2(6.0f * tempscale);

		buttonY->position = vec2(24.0f * tempscale, -20.0f * tempscale);
		buttonY->scale = vec2(6.0f * tempscale);

		// D-pad

		dpadUp->position = vec2(-24.0f * tempscale, -18.0f * tempscale);
		dpadUp->scale = vec2(7.0f * tempscale);

		dpadDown->position = vec2(-24.0f * tempscale, -10.0f * tempscale);
		dpadDown->rotation = radians(180.0f);
		dpadDown->scale = vec2(7.0f * tempscale);

		dpadLeft->position = vec2(-28.0f * tempscale, -14.0f * tempscale);
		dpadLeft->rotation = radians(90.0f);
		dpadLeft->scale = vec2(7.0f * tempscale);

		dpadRight->position = vec2(-20.0f * tempscale, -14.0f * tempscale);
		dpadRight->rotation = radians(270.0f);
		dpadRight->scale = vec2(7.0f * tempscale);

		// Burgers :)
		
		bumperLeft->position = vec2(-22.0f * tempscale, -30.0f * tempscale);
		bumperLeft->scale = vec2(15.0f * tempscale);
		bumperRight->position = vec2(22.0f * tempscale, -30.0f * tempscale);
		bumperRight->scale = vec2(15.0f * tempscale);

		scale = vec2(500.0f, -500.0f);
		markDirty();
	}

	void initHiearchy() {
		// Add children
		addChild(leftStick);
		addChild(rightStick);
		addChild(buttonA);
		addChild(buttonB);
		addChild(buttonX);
		addChild(buttonY);
		addChild(dpadUp);
		addChild(dpadDown);
		addChild(dpadLeft);
		addChild(dpadRight);
		addChild(bumperLeft);
		addChild(bumperRight);
	}

	void updateFromInput(const GamepadInput& input) {
		leftStick->setStickPosition(input.leftStick);
		rightStick->setStickPosition(input.rightStick);
		leftStick->setPressed(input.leftStickPressed);
		rightStick->setPressed(input.rightStickPressed);

		buttonA->setPressed(input.buttonA);
		buttonB->setPressed(input.buttonB);
		buttonX->setPressed(input.buttonX);
		buttonY->setPressed(input.buttonY);

		dpadUp->setPressed(input.dpad[1] > 0);
		dpadDown->setPressed(input.dpad[1] < 0);
		dpadLeft->setPressed(input.dpad[0] < 0);
		dpadRight->setPressed(input.dpad[0] > 0);

		bumperLeft->setPressed(input.leftBumper);
		bumperRight->setPressed(input.rightBumper);
	}

	virtual void update(double dt) override {
		leftStick->update(dt);
		rightStick->update(dt);
	}
};