#pragma once
#include "Transform2D.h"
#include "SpriteRenderer.h"
#include "GLFW/glfw3.h"

using namespace std;
using namespace glm;


class AnalogStickObject : public Transform2D {
	unsigned int stickHead;
	unsigned int stickHeadPressed;

	bool isPressed = false;
	vec2 stickPosition = vec2(0.0f);
public:
	vec2 displacementFactor = vec2(0.5f); // Multiplier for stick displacement
	double centeringSpeed = 5.0f; // Speed at which the stick returns to center when released

	function<void(int)> onGlfwEvent;

	AnalogStickObject(unsigned int headTex, unsigned int pressedTex)
		: stickHead(headTex), stickHeadPressed(pressedTex) {
	}

	bool containsPoint(vec2 point) {
		vec2 pos = getWorldPosition();
		vec2 size = getWorldScale();


		// Circle check (assuming size.x is diameter)
		vec2 center = pos + vec2(size.x / 2.0f, size.y / 2.0f);
		float radius = size.x / 2.0f;
		return length(point - center) <= radius;

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


	void setStickPosition(vec2 pos) {
		// Clamp position to unit circle
		if (length(pos) > 1.0f) {
			pos = normalize(pos);
		}
		stickPosition = pos;
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
	


	void Update(double dt) override {
		if (!isPressed) {
			// Gradually return stick to center position
			stickPosition = stickPosition - stickPosition * (float)(centeringSpeed * dt);
			if (length(stickPosition) < 0.01f) {
				stickPosition = vec2(0.0f);
			}
		}
	}

	// ---------- Rendering ---------- 

	unsigned int getCurrentTexture() const {
		return isPressed ? stickHead : stickHeadPressed;
	}

	void Draw(SpriteRenderer& renderer) override {
		renderer.Draw(getCurrentTexture(), getOffsetWorldMatrix());
	}
};