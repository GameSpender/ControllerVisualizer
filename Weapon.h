#pragma once
#include "Transform2D.h"


enum WeaponType {
	LaserGun, Minigun
};

class Weapon : public Transform2D{
public:
	double shotInterval;
	double nextShot;

	virtual void startFiring() = 0;
	virtual void stopFiring() = 0;

};