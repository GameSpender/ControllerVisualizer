#pragma once
#include "Transform2D.h"
#include "SpriteRenderer.h"
#include "GLFW/glfw3.h"
#include "InteractionInterfaces.h"

using namespace std;
using namespace glm;

enum ButtonShape {
	RECTANGLE,
	CIRCLE
};

class ButtonObject : public Transform2D, public Interactive {
	unsigned int textureIdle;
	unsigned int texturePressed;

	ButtonShape shape = RECTANGLE;

	bool isPressed = false;
public:

	function<void(int)> onButtonEvent;

	ButtonObject(unsigned int idleTex, unsigned int pressedTex, ButtonShape shape = RECTANGLE)
		: textureIdle(idleTex), texturePressed(pressedTex), shape(shape) {
	}

	Interactive* hitTest(vec2 point) override {
		vec2 pos = getWorldPosition();
		vec2 size = getWorldScale();

		if(shape == RECTANGLE) {
			// AABB check
			return (point.x >= pos.x - size.x / 2 && point.x <= pos.x + size.x / 2 &&
					point.y >= pos.y - size.y / 2 && point.y <= pos.y + size.y / 2) ? this : nullptr;
		} else if(shape == CIRCLE) {
			// Circle check (assuming size.x is diameter)
			vec2 center = pos;
			float radius = size.x / 2.0f;
			return length(point - center) <= radius ? this : nullptr;
		}
	}

	void onMouseInput(int button, int inputType) override {
		if(button != GLFW_MOUSE_BUTTON_LEFT)
			return; // only care about left mouse button
		if (inputType == GLFW_PRESS) {
			setPressed(true);
		}
		else if (inputType == GLFW_RELEASE) {
			setPressed(false);
		}
	}


	void setPressed(bool pressed) {
		if (pressed == isPressed)
			return; // no state change

		isPressed = pressed;

		// Emit GLFW-like input event
		if (onButtonEvent) {
			onButtonEvent(isPressed ? GLFW_PRESS : GLFW_RELEASE);
		}
	}

	bool Pressed() const {
		return isPressed;
	}

	unsigned int getCurrentTexture() const {
		return isPressed ? texturePressed : textureIdle;
	}

	void Draw(SpriteRenderer& renderer) override {
		renderer.Draw(getCurrentTexture(), getWorldMatrix());
	}
};