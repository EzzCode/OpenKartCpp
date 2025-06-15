#pragma once

#include "../ecs/world.hpp"
#include "../components/sound.hpp"
#include "miniaudio.h"
#include <vector>
#include <iostream>
namespace our
{

    // The sound system is responsible for moving every entity which contains a soundComponent.
    // This system is added as a simple example for how use the ECS framework to implement logic.
    // For more information, see "common/components/sound.hpp"
    class soundSystem
    {
    private:
        ma_engine engine;
        // Store pointers to heap-allocated sounds for proper cleanup
        std::vector<ma_sound *> oneShotSounds;

    public:
        void initialize()
        {
            ma_result result = ma_engine_init(NULL, &engine);
            if (result != MA_SUCCESS)
            {
                std::cerr << "Failed to initialize audio engine." << std::endl;
            }
            else
            {
                std::cout << "Audio engine initialized successfully." << std::endl;
            }
        } // This should be called every frame to update all entities containing a soundComponent.
        void update(World *world, float deltaTime)
        {
            // Clean up finished one-shot sounds
            cleanupFinishedSounds();

            // For each entity in the world
            for (auto entity : world->getEntities())
            {
                // Get the sound component if it exists
                SoundComponent *soundComp = entity->getComponent<SoundComponent>();
                // If the sound component exists
                if (soundComp)
                {
                    if (!soundComp->playing)
                    {
                        const char *soundFile = soundComp->soundPath.c_str();
                        ma_result result = ma_sound_init_from_file(&engine, soundFile, MA_SOUND_FLAG_DECODE, NULL, NULL, &soundComp->sound);
                        if (result != MA_SUCCESS)
                        {
                            std::cerr << "Failed to initialize sound from file." << std::endl;
                        }
                        else
                        {
                            ma_sound_set_volume(&soundComp->sound, soundComp->volume / 100.0);
                            if (soundComp->looped)
                            {
                                ma_sound_set_looping(&soundComp->sound, true); // Loop the sound
                            }
                            ma_sound_start(&soundComp->sound);
                            soundComp->playing = true;
                        }
                    }
                }
            }
        }
        void destroy()
        {
            // Clean up all remaining one-shot sounds
            for (ma_sound *sound : oneShotSounds)
            {
                if (sound)
                {
                    ma_sound_uninit(sound);
                    delete sound;
                }
            }
            oneShotSounds.clear();

            ma_engine_uninit(&engine);
        }

    private:
        void cleanupFinishedSounds()
        {
            // Remove finished one-shot sounds
            auto it = oneShotSounds.begin();
            while (it != oneShotSounds.end())
            {
                ma_sound *sound = *it;
                if (sound && !ma_sound_is_playing(sound))
                {
                    ma_sound_uninit(sound);
                    delete sound;
                    it = oneShotSounds.erase(it);
                }
                else
                {
                    ++it;
                }
            }
        }

    public:
        // Play a one-shot sound effect
        void playSound(const std::string &soundPath, float volume = 1.0f)
        {
            std::cout << "Attempting to play sound: " << soundPath << std::endl;

            // Clean up finished sounds before adding new ones
            cleanupFinishedSounds();

            // Create a new sound on the heap
            ma_sound *sound = new ma_sound();
            ma_result result = ma_sound_init_from_file(&engine, soundPath.c_str(),
                                                       MA_SOUND_FLAG_DECODE | MA_SOUND_FLAG_ASYNC, NULL, NULL, sound);

            if (result != MA_SUCCESS)
            {
                std::cerr << "Failed to initialize one-shot sound from file: " << soundPath
                          << " Error code: " << result << std::endl;
                delete sound;
                return;
            }

            ma_sound_set_volume(sound, volume);
            result = ma_sound_start(sound);

            if (result != MA_SUCCESS)
            {
                std::cerr << "Failed to start sound: " << soundPath
                          << " Error code: " << result << std::endl;
                ma_sound_uninit(sound);
                delete sound;
                return;
            }

            std::cout << "Sound started successfully: " << soundPath << std::endl;

            // Add to our tracking list for cleanup
            oneShotSounds.push_back(sound);
        }
    };

}
