#pragma once
#include "irrKlang/irrKlang.h"
#include <string>
#include <unordered_map>
#include <memory>
#include <iostream>
#include <iostream>

using namespace irrklang;

class SoundManager {
    // Maps object ID -> list of currently playing ISound* for that object
    std::unordered_map<void*, std::vector<ISound*>> objectSounds;

public:
    SoundManager() {
        engine = createIrrKlangDevice();
        if (!engine) {
            std::cerr << "Failed to create irrKlang engine!" << std::endl;
        }
    }

    ~SoundManager() {
        if (engine) {
            engine->drop();
        }
    }

    // Preload 
    void loadSound(const std::string& name, const std::string& filePath, bool streamed = false) {
        if (!engine) return;

        if (sounds.find(name) != sounds.end())
            return; // already loaded

        ISoundSource* src = engine->addSoundSourceFromFile(filePath.c_str(), streamed ? ESM_STREAMING : ESM_AUTO_DETECT, true);
        if (!src) {
            std::cerr << "Failed to load sound: " << filePath << std::endl;
            return;
        }

        std::cout << "Loaded sound: " << filePath << std::endl;
        sounds[name] = src;
    }

    // Retrieve sound
    ISoundSource* getSound(const std::string& name) {
        auto it = sounds.find(name);
        if (it != sounds.end())
            return it->second;

        std::cerr << "Sound not found: " << name << std::endl;
        return nullptr;
    }

    // Play a sound by name, optionally starting at a specific time (seconds)
    ISound* play(const std::string& name, float volume = 1.0f, bool loop = false, float startTime = 0.0f) {
        if (!engine) return nullptr;

        ISoundSource* src = getSound(name);
        if (!src) return nullptr;

        // Start paused so we can set the playback position
        ISound* s = engine->play2D(src, loop, true, true);
        if (s) {
            s->setVolume(volume);

            // Convert seconds to milliseconds
            if (startTime > 0.0f) {
                s->setPlayPosition(static_cast<ik_u32>(startTime * 1000.0f));
            }

            s->setIsPaused(false); // unpause to start playback
        }

        return s;
    }


    // Stop a specific instance of a sound
    void stop(ISound* sound) {
        if (sound) {
            sound->stop();
            sound->drop();   // cleanup reference
        }
    }

    ISound* playForObject(
        void* object,
        const std::string& name,
        float volume = 1.0f,
        bool loop = false,
        float startTimeSeconds = 0.0f,
        float playbackSpeed = 1.0f
    ) {
        if (!engine) return nullptr;

        ISoundSource* src = getSound(name);
        if (!src) return nullptr;

        // Play paused so we can set position
        ISound* s = engine->play2D(src, loop, true, true);
        if (!s) return nullptr;

        s->setVolume(volume);

        // Set speed
        if (playbackSpeed != 1.0f)
            s->setPlaybackSpeed(playbackSpeed);

        // Set start position in milliseconds
        if (startTimeSeconds > 0.0f) {
            float startMs = startTimeSeconds * 1000.0f;
            // Clamp to sound length
            startMs = std::min(startMs, (float)s->getPlayLength());
            std::cout << s->setPlayPosition(startMs) << " " << startMs << std::endl;
        }

        // Unpause to start playback
        s->setIsPaused(false);

        objectSounds[object].push_back(s);
        return s;
    }

    void stopForObject(void* object) {
        auto it = objectSounds.find(object);
        if (it != objectSounds.end()) {
            for (ISound* s : it->second) {
                if (s) s->stop();
            }
            it->second.clear();
        }
    }


    // Stop everything currently playing
    void stopAll() {
        if (engine) {
            engine->stopAllSounds();
        }
    }

    // Global engine volume
    void setMasterVolume(float volume) {
        if (engine) engine->setSoundVolume(volume);
    }

    float getMasterVolume() const {
        return engine ? engine->getSoundVolume() : 0.0f;
    }

private:
    ISoundEngine* engine = nullptr;
    std::unordered_map<std::string, ISoundSource*> sounds;
};
