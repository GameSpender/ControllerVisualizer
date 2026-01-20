#pragma once
#include "GLFW/glfw3.h"
#include <stdio.h>
#include "glm/ext.hpp"

#include "Actor.h"

using namespace std;
using namespace glm;


class AnalogStickObject : public Actor2D{
	bool isPressed = false;
	bool isHeld = false;
	vec2 stickPosition = vec2(0.0f);
public:
	string stickHead = "stick_head";
	string stickHeadPressed = "stick_head_pressed";

	vec2 displacementFactor = vec2(1.0f); // Multiplier for stick displacement
	double centeringSpeed = 5.0f; // Speed at which the stick returns to center when released

	AnalogStickObject()
		: isPressed(false) {
		spriteName = stickHead;
	}

	void setPressed(bool pressed) {
		if (pressed == isPressed)
			return; // no state change

		isPressed = pressed;

		spriteName = isPressed ? stickHeadPressed : stickHead;
	}

	bool Pressed() const {
		return isPressed;
	}

	void setOrigin(vec2 pos) {
		stickPosition = pos;
		position = stickPosition;
	}

	void setStickPosition(vec2 pos) {
		// Clamp position to unit circle
		if (length(pos) > 1.0f) {
			pos = normalize(pos);
		}
		position = stickPosition + pos * displacementFactor * scale;
		markDirty();
	}

};