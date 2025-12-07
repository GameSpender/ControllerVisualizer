#pragma once
#include "Actor.h"
#include "EventBus.h"

enum WeaponType {
	LaserGun, Minigun
};

struct WeaponFiredEvent {
	vec2 position;
	vec2 direction;
	WeaponType type;
};


class Weapon : public Actor2D{
	
public:
	double shotInterval;
	double nextShot;


	virtual void startFiring() = 0;
	virtual void stopFiring() = 0;

};

