#pragma once
#include "SpriteRenderer.h"
#include "GLFW/glfw3.h"
#include "Actor.h"

using namespace std;
using namespace glm;

enum ButtonShape {
	RECTANGLE,
	CIRCLE
};

class ButtonObject : public Actor2D {
	ButtonShape shape = RECTANGLE;

	bool isPressed = false;
public:
	string buttonSprite = "button";
	string buttonPressedSprite = "button_pressed";

	ButtonObject(ButtonShape shape = CIRCLE)
		: shape(shape) {
		spriteName = buttonSprite;
	}

	ButtonObject(string buttonSprite, string buttonPressedSprite, ButtonShape shape = CIRCLE)
		: buttonSprite(buttonSprite), buttonPressedSprite(buttonPressedSprite) , shape(shape) {
		spriteName = buttonSprite;
	}

	void setPressed(bool pressed) {
		if (pressed == isPressed)
			return; // no state change

		isPressed = pressed;
		spriteName = isPressed ? buttonPressedSprite : buttonSprite;
	}

	bool Pressed() const {
		return isPressed;
	}

};