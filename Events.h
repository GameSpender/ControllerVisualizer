#pragma once
#include "glm/glm.hpp"
#include "Transform3D.h"

using namespace glm;

struct MoveEvent {
    vec3 position;
    vec3 direction;
    float speed;
};

struct ShootEvent {
    vec3 position;             // world position of muzzle
    vec3 direction;            // normalized firing direction
    std::string projectileType; // e.g. "Laser", "Missile"
    std::string soundName;      // which sound to play
    std::string effectName;     // muzzle flash effect
};

struct SoundEvent {
    void* owner = nullptr;          // object producing the sound (e.g., weapon)
    std::string soundName;          // name of the sound to play
    bool loop = false;              // should it loop?
    float volume = 1.0f;            // volume
    bool stop = false;              // if true, stop the sound instead of playing
    float startTime = 0.0f;         // optional playback offset
};

struct DamageEvent {
    void* target = nullptr; // pointer to the owning actor/component
    float amount = 0.0f;
    int team = 0;
};

struct DeathEvent {
    void* target = nullptr;
    int team = 0;
};

struct RespawnEvent {
    void* target = nullptr;
    int team = 0;
};
