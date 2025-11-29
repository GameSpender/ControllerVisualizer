#pragma once
#include "Transform2D.h"

#include <GL/glew.h>
#include <glm/glm.hpp>

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
};;

class GamepadObject : public Transform2D, public Interactive, public Animated{

	unsigned int gamepadBody;
public:
	AnalogStickObject leftStick;
	AnalogStickObject rightStick;

	ButtonObject buttonA;
	ButtonObject buttonB;
	ButtonObject buttonX;
	ButtonObject buttonY;

	ButtonObject dpadUp;
	ButtonObject dpadDown;
	ButtonObject dpadLeft;
	ButtonObject dpadRight;

public:

	GamepadObject(GamepadTextures tex):
		gamepadBody(tex.gamepadBody),
		leftStick(tex.stickHead, tex.stickHeadPressed), rightStick(tex.stickHead, tex.stickHeadPressed),
		buttonA(tex.buttonAIdle, tex.buttonAPressed, CIRCLE),
		buttonB(tex.buttonBIdle, tex.buttonBPressed, CIRCLE),
		buttonX(tex.buttonXIdle, tex.buttonXPressed, CIRCLE),
		buttonY(tex.buttonYIdle, tex.buttonYPressed, CIRCLE),
		dpadUp(tex.dpadIdle, tex.dpadPressed, RECTANGLE),
		dpadDown(tex.dpadIdle, tex.dpadPressed, RECTANGLE),
		dpadLeft(tex.dpadIdle, tex.dpadPressed, RECTANGLE),
		dpadRight(tex.dpadIdle, tex.dpadPressed, RECTANGLE)
	{// Setup hierarchy
		addChild(&leftStick);
		addChild(&rightStick);
		addChild(&buttonA);
		addChild(&buttonB);
		addChild(&buttonX);
		addChild(&buttonY);
		addChild(&dpadUp);
		addChild(&dpadDown);
		addChild(&dpadLeft);
		addChild(&dpadRight);
		// Position components appropriately (example positions, adjust as needed)
		double tempscale = 1.0f / 100.0f;

		leftStick.position = vec2(-12.0f * tempscale, 0.0f * tempscale);
		leftStick.scale = vec2(13.0f * tempscale);
		rightStick.position = vec2(12.0f * tempscale, 0.0f * tempscale);
		rightStick.scale = vec2(13.0f * tempscale);

		// ABXY buttons

		buttonA.position = vec2(24.0f * tempscale, -8.0f * tempscale);
		buttonA.scale = vec2(6.0f * tempscale);

		buttonB.position = vec2(30.0f * tempscale, -14.0f * tempscale);
		buttonB.scale = vec2(6.0f * tempscale);

		buttonX.position = vec2(18.0f * tempscale, -14.0f * tempscale);
		buttonX.scale = vec2(6.0f * tempscale);

		buttonY.position = vec2(24.0f * tempscale, -20.0f * tempscale);
		buttonY.scale = vec2(6.0f * tempscale);

		// D-pad

		dpadUp.position = vec2(-24.0f * tempscale, -18.0f * tempscale);
		dpadUp.scale = vec2(7.0f * tempscale);

		dpadDown.position = vec2(-24.0f * tempscale, -10.0f * tempscale);
		dpadDown.rotation = radians(180.0f);
		dpadDown.scale = vec2(7.0f * tempscale);

		dpadLeft.position = vec2(-28.0f * tempscale, -14.0f * tempscale);
		dpadLeft.rotation = radians(90.0f);
		dpadLeft.scale = vec2(7.0f * tempscale);

		dpadRight.position = vec2(-20.0f * tempscale, -14.0f * tempscale);
		dpadRight.rotation = radians(270.0f);
		dpadRight.scale = vec2(7.0f * tempscale);

		scale = vec2(500.0f);
		markDirty();
	}

	virtual Interactive* hitTest(vec2 world) override {
		for (Transform2D* child : children) {
			Interactive* hitTestableChild = dynamic_cast<Interactive*>(child);
			if (hitTestableChild && hitTestableChild->hitTest(world)) {
				return hitTestableChild;
			}
		}

		return nullptr;
	}

	virtual void onMouseInput(int button, int inputType) override {
		if (inputType == GLFW_RELEASE) {
						// Propagate to all children
			for (Transform2D* child : children) {
				Interactive* interactiveChild = dynamic_cast<Interactive*>(child);
				if (interactiveChild) {
					interactiveChild->onMouseInput(button, inputType);
				}
			}
		}
	}

	virtual void onMouseMove(vec2 mousePos) override {
		// Propagate to all children
		leftStick.onMouseMove(mousePos);
		rightStick.onMouseMove(mousePos);
	}

	virtual void update(double dt) override {
		leftStick.update(dt);
		rightStick.update(dt);
	}

	void Draw(SpriteRenderer& renderer) override{
		// Draw gamepad body
		renderer.Draw(gamepadBody, getWorldMatrix());
		// Draw components
		leftStick.Draw(renderer);
		rightStick.Draw(renderer);
		buttonA.Draw(renderer);
		buttonB.Draw(renderer);
		buttonX.Draw(renderer);
		buttonY.Draw(renderer);
		dpadUp.Draw(renderer);
		dpadDown.Draw(renderer);
		dpadLeft.Draw(renderer);
		dpadRight.Draw(renderer);
	}

};