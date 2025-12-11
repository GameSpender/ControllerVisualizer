#pragma once
#include "EventBus.h"
#include "Events.h"
#include "SoundManager.h"
#include "ProjectileSystem.h"
#include "Services.h"


class EventHandler {

public:

	EventHandler(){}


	void processEvents() {
		if (!Services::eventBus) return;

		Services::eventBus->process<ShootEvent>([&](const ShootEvent shot) {
			if (!Services::sound) return;
			if (shot.projectileType == "laser_shot") {
				Services::sound->play(shot.soundName);
			}
		});

		Services::eventBus->process<SoundEvent>([&](const SoundEvent sound) {
			if (!Services::sound) return;
			if (sound.stop) {
				Services::sound->stopForObject(sound.owner);
			}
			else {
				Services::sound->playForObject(sound.owner, sound.soundName, sound.volume, sound.loop, sound.startTime);
			}
				
		});
	}

};