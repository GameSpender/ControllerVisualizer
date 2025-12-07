#pragma once
#include "EventBus.h"
#include "Events.h"
#include "SoundManager.h"
#include "ProjectileSystem.h"


class EventHandler {
	SoundManager* soundManager;
	ProjectileSystem* projectileSystem;
	EventBus* bus;
public:

	EventHandler(EventBus* bus) : bus(bus) {}

	void init(SoundManager* soundMan, ProjectileSystem* proj) {
		soundManager = soundMan;
		projectileSystem = proj;
	}


	void processEvents() {
		bus->process<ShootEvent>([&](const ShootEvent shot) {
			if (!soundManager) return;
			if (shot.projectileType == "laser_shot") {
				soundManager->play(shot.soundName);
			}
		});

		bus->process<SoundEvent>([&](const SoundEvent sound) {
			if (!soundManager) return;
			if (sound.stop) {
				soundManager->stopForObject(sound.owner);
			}
			else {
				soundManager->playForObject(sound.owner, sound.soundName, sound.volume, sound.loop, sound.startTime);
			}
				
		});
	}

};