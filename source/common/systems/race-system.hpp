#pragma once

#include "../ecs/world.hpp"
#include "../components/race.hpp"
#include "../application.hpp"
#include <glm/glm.hpp>
#include <vector>
#include <algorithm>

namespace our {

    class RaceSystem {
    private:
        Application* app;
        Entity* raceManager;
        std::vector<Entity*> players;
        std::vector<Entity*> checkpoints;
        float deltaTime;
        
        // UI and HUD functions
        void updateCountdown(RaceManagerComponent* manager, float dt);
        void checkPlayerProgress(Entity* player, RacePlayerComponent* racePlayer);
        void updateRacePositions();
        float calculateDistanceToNextCheckpoint(Entity* player, RacePlayerComponent* racePlayer);
        
    public:
        // Initialize the race system
        void enter(Application* app);
        
        // Main update function called every frame
        void update(World* world, float deltaTime);
        
        // Start the race
        void startRace();
        
        // Reset race to beginning
        void resetRace();
        
        // Get race statistics for display
        bool isRaceActive() const;
        bool isRaceCompleted() const;
        float getRaceTime() const;
        int getCurrentLap() const;
        int getPosition() const;
        float getBestLapTime() const;
        std::string getRaceStateString() const;
    };

}
