#pragma once
#include "Actor3D.h"
#include "BaseComponent3D.h"


class LifetimeComponent : public BaseComponent3D {
public:


	float lifetime = 1.0f; // seconds

	LifetimeComponent(float life) : lifetime(life) {}

	void update(double dt) override {

		lifetime -= dt;

		if (owner && lifetime < 0) {
			owner->markDeleted();
		}
	}
};