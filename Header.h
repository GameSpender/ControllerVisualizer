#pragma once
#include "Actor3D.h"

class TestProjectileActor : public Actor3D {
public:
	TestProjectileActor() {
		name = "TestProjectile";
	}

	void update(double dt) override {
		Actor3D::update(dt);
		// Move forward at constant speed
		float speed = 10.0f; // units per second
		position += forward() * speed * static_cast<float>(dt);
		markDirty();
	}
};