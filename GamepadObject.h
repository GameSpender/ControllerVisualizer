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
#include "InteractionInterfaces.h"


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

class GamepadObject : public Transform2D, public Interactive, public Animated{

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

	GamepadObject(GamepadTextures tex):
		gamepadBody(tex.gamepadBody)
	{// Setup hierarchy
		leftStick = std::make_shared<AnalogStickObject>(tex.stickHead, tex.stickHeadPressed);
		rightStick = std::make_shared<AnalogStickObject>(tex.stickHead, tex.stickHeadPressed);

		buttonA = std::make_shared<ButtonObject>(tex.buttonAIdle, tex.buttonAPressed, CIRCLE);
		buttonB = std::make_shared<ButtonObject>(tex.buttonBIdle, tex.buttonBPressed, CIRCLE);
		buttonX = std::make_shared<ButtonObject>(tex.buttonXIdle, tex.buttonXPressed, CIRCLE);
		buttonY = std::make_shared<ButtonObject>(tex.buttonYIdle, tex.buttonYPressed, CIRCLE);

		dpadUp = std::make_shared<ButtonObject>(tex.dpadIdle, tex.dpadPressed, RECTANGLE);
		dpadDown = std::make_shared<ButtonObject>(tex.dpadIdle, tex.dpadPressed, RECTANGLE);
		dpadLeft = std::make_shared<ButtonObject>(tex.dpadIdle, tex.dpadPressed, RECTANGLE);
		dpadRight = std::make_shared<ButtonObject>(tex.dpadIdle, tex.dpadPressed, RECTANGLE);

		bumperLeft = std::make_shared<ButtonObject>(tex.bumper, tex.bumperPressed, RECTANGLE);
		bumperRight = std::make_shared<ButtonObject>(tex.bumper, tex.bumperPressed, RECTANGLE);

		// Position components appropriately (example positions, adjust as needed)
		double tempscale = 1.0f / 100.0f;

		leftStick->position = vec2(-12.0f * tempscale, 0.0f * tempscale);
		leftStick->scale = vec2(10.0f * tempscale);
		leftStick->displacementFactor = vec2(0.4f);
		rightStick->position = vec2(12.0f * tempscale, 0.0f * tempscale);
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

		// Bumpers
		
		bumperLeft->position = vec2(-22.0f * tempscale, -30.0f * tempscale);
		bumperLeft->scale = vec2(15.0f) * tempscale;
		bumperRight->position = vec2(22.0f * tempscale, -30.0f * tempscale);
		bumperRight->scale = vec2(15.0f) * tempscale;

		scale = vec2(500.0f);
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

	virtual Interactive* hitTest(vec2 world) override {
		for (auto child : children) {
			auto hitTestableChild = dynamic_pointer_cast<Interactive>(child);
			if (hitTestableChild && hitTestableChild->hitTest(world)) {
				return hitTestableChild.get();
			}
		}

		return nullptr;
	}

	virtual void onMouseInput(int button, int inputType) override {
		if (inputType == GLFW_RELEASE) {
						// Propagate to all children
			for (auto child : children) {
				auto interactiveChild = dynamic_pointer_cast<Interactive>(child);
				if (interactiveChild) {
					interactiveChild->onMouseInput(button, inputType);
				}
			}
		}
	}

	virtual void onMouseMove(vec2 mousePos) override {
		leftStick->onMouseMove(mousePos);
		rightStick->onMouseMove(mousePos);
	}

	virtual void update(double dt) override {
		leftStick->update(dt);
		rightStick->update(dt);
	}

	void Draw(SpriteRenderer& renderer) override{
		renderer.Draw(gamepadBody, getWorldMatrix());

		leftStick->Draw(renderer);
		rightStick->Draw(renderer);
		buttonA->Draw(renderer);
		buttonB->Draw(renderer);
		buttonX->Draw(renderer);
		buttonY->Draw(renderer);
		dpadUp->Draw(renderer);
		dpadDown->Draw(renderer);
		dpadLeft->Draw(renderer);
		dpadRight->Draw(renderer);
		bumperLeft->Draw(renderer);
		bumperRight->Draw(renderer);
	}

};