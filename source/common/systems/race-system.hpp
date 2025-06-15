#pragma once

#include "../ecs/world.hpp"
#include "../components/race.hpp"
#include "../components/sound.hpp"
#include "../application.hpp"
#include <glm/glm.hpp>
#include <vector>
#include <algorithm>

namespace our
{
    // Forward declaration
    class soundSystem;


  class RaceSystem
    {
    private:
        Application *app;
        Entity *raceManager;
        std::vector<Entity *> players;
        std::vector<Entity *> checkpoints;
        float deltaTime;
        bool systemInitialized = false;
        float initializationTime = 0.0f;
        int lastCountdownValue = 4;
        soundSystem *soundSystemRef = nullptr; // Reference to sound system

        // UI and HUD functions
        void updateCountdown(RaceManagerComponent *manager, float dt);
        void checkPlayerProgress(Entity *player, RacePlayerComponent *racePlayer);
        void updateRacePositions();
        void updateCheckpointVisibility();
        float calculateDistanceToNextCheckpoint(Entity *player, RacePlayerComponent *racePlayer);
        
        // Sound functions
        void playCheckpointSound();
        void playLapCompleteSound();
        void playRaceCompleteSound();    public:
        // Initialize the race system
        void enter(Application *app);
        
        // Set sound system reference for audio feedback
        void setSoundSystem(soundSystem *soundSys);

        // Main update function called every frame
        void update(World *world, float deltaTime);

        // Start the race
        void startRace();

        // Reset race to beginning
        void resetRace();
        // Get race statistics for display
        bool isRaceActive() const;
        bool isRaceCompleted() const;
        bool isInputDisabled() const;
        float getRaceTime() const;
        int getCurrentLap() const;
        int getPosition() const;
        float getBestLapTime() const;
        std::string getRaceStateString() const;
        int getSpeed() const;
    };
}
