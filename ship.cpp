#include <corecrt_math.h>






class Ship {
public:
	float position[2];
	float rotation;
	float inertia[3] = { 0, 0 };
	
	float thrustDir[2];
	float thrustRot;

	float mass = 5;
	float rotationFriction = 1;
	float friction = 1;

	


	Ship(float position[2], float rotation) {
		for (int i = 0; i < 2; i++) {
			this->position[i] = position[i];
			this->rotation = rotation;
		}
	}

	Ship(float x, float y, float rotation) : rotation(rotation) {
		position[0] = x;
		position[1] = y;
	}


	void update(float delta) {
		thrust(thrustDir, delta);
		thrustRotate(thrustRot, delta);

		for (int i = 0; i < 2; i++) {
			position[i] += inertia[i] * delta;
			inertia[i] -= inertia[i] * friction * delta;
		}
		rotation += inertia[2] * delta;
		inertia[2] -= inertia[2] * rotationFriction * delta;
	}

private:
	void thrust(float thrustVec[2], float delta) {
		// Convert local thrust to world space
		float c = cos(rotation);
		float s = sin(rotation);

		float worldX = thrustVec[0] * c - thrustVec[1] * s;
		float worldY = thrustVec[0] * s + thrustVec[1] * c;

		inertia[0] += worldX * delta;
		inertia[1] += worldY * delta;
	}


	void thrustRotate(float thrustVec, float delta) {
		inertia[2] += thrustVec * delta;
	}
};