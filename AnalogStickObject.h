#pragma once
#include "Transform2D.h"
#include "SpriteRenderer.h"
#include "GLFW/glfw3.h"
#include "InteractionInterfaces.h"

using namespace std;
using namespace glm;


class AnalogStickObject : public Transform2D, public Interactive, public Animated {
	unsigned int stickHead;
	unsigned int stickHeadPressed;

	bool isPressed = false;
	bool isHeld = false;
	vec2 stickPosition = vec2(0.0f);
public:
	vec2 displacementFactor = vec2(0.5f); // Multiplier for stick displacement
	double centeringSpeed = 5.0f; // Speed at which the stick returns to center when released

	function<void(int)> onButtonEvent;
	function<void(vec2)> onStickEvent;

	AnalogStickObject(unsigned int headTex, unsigned int pressedTex)
		: stickHead(headTex), stickHeadPressed(pressedTex), isPressed(false) {
	}

	Interactive* hitTest(vec2 point) override {
		vec2 pos = getWorldPosition();
		vec2 size = getWorldScale();


		// Circle check (assuming size.x is diameter)
		vec2 center = pos + (stickPosition * displacementFactor);
		float radius = size.x / 2.0f;
		return length(point - center) <= radius ? this : nullptr;

	}

	void onMouseInput(int button, int inputType) override {
		if (button == GLFW_MOUSE_BUTTON_LEFT) {
			if (inputType == GLFW_PRESS) {
				setPressed(true);
			}
			else if (inputType == GLFW_RELEASE) {
				setPressed(false);
			}
		}
		else if (button == GLFW_MOUSE_BUTTON_RIGHT) {
			if (inputType == GLFW_PRESS) {
				isHeld = true;
			}
			else if (inputType == GLFW_RELEASE) {
				isHeld = false;
			}
		}
		
	}

	void onMouseMove(vec2 mousePos) override {
		if (!isHeld) return;

		vec2 pos = getWorldPosition();
		vec2 size = getWorldScale();

		vec2 center = pos;
		vec2 dir = mousePos - center;

		vec2 norm = dir / (size * displacementFactor);

		setStickPosition(norm);
	}


	void update(double dt) override {
		if (!isHeld) {
			setStickPosition(stickPosition - stickPosition * (float)(centeringSpeed * dt));
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


	void setStickPosition(vec2 pos) {
		// Clamp position to unit circle
		if (length(pos) > 1.0f) {
			pos = normalize(pos);
		}
		stickPosition = pos;

		if(onStickEvent)
			onStickEvent(stickPosition);
	}

	mat3 getOffsetWorldMatrix() {
		mat3 P = getWorldMatrix();
		
		mat3 T = mat3(1.0f);
		T[2] = vec3(stickPosition * displacementFactor * scale, 0.0f);

		mat3 R = mat3(
			cos(rotation), -sin(rotation), 0,
			sin(rotation), cos(rotation), 0,
			0, 0, 1
		);



		return T * R * P;
	}
	

	// ---------- Rendering ---------- 

	unsigned int getCurrentTexture() const {
		return isPressed ? stickHeadPressed : stickHead;
	}

	void Draw(SpriteRenderer& renderer) override {
		renderer.Draw(getCurrentTexture(), getOffsetWorldMatrix());
	}
};