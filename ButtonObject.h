#pragma once
#include "Transform2D.h"
#include "SpriteRenderer.h"
#include "GLFW/glfw3.h"

using namespace std;
using namespace glm;

enum ButtonShape {
	RECTANGLE,
	CIRCLE
};

class ButtonObject : public Transform2D {
	unsigned int textureIdle;
	unsigned int texturePressed;

	ButtonShape shape = RECTANGLE;

	bool isPressed = false;
public:

	function<void(int)> onGlfwEvent;

	ButtonObject(unsigned int idleTex, unsigned int pressedTex, ButtonShape shape = RECTANGLE)
		: textureIdle(idleTex), texturePressed(pressedTex) {
	}

	bool containsPoint(vec2 point) {
		vec2 pos = getWorldPosition();
		vec2 size = getWorldScale();

		if(shape == RECTANGLE) {
			// AABB check
			return (point.x >= pos.x && point.x <= pos.x + size.x &&
					point.y >= pos.y && point.y <= pos.y + size.y);
		} else if(shape == CIRCLE) {
			// Circle check (assuming size.x is diameter)
			vec2 center = pos + vec2(size.x / 2.0f, size.y / 2.0f);
			float radius = size.x / 2.0f;
			return length(point - center) <= radius;
		}
	}

	void setPressed(bool pressed) {
		if (pressed == isPressed)
			return; // no state change

		isPressed = pressed;

		// Emit GLFW-like input event
		if (onGlfwEvent) {
			onGlfwEvent(isPressed ? GLFW_PRESS : GLFW_RELEASE);
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