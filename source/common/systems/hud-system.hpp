#pragma once

#include "../ecs/world.hpp"
#include "../application.hpp"
#include "race-system.hpp"
#include <string>

namespace our {

    class HUDSystem {
    private:
        Application* app;
        RaceSystem* raceSystem;
        
        // Display variables
        bool showHUD = true;
        float fontSize = 24.0f;        // Helper functions for text rendering
        void renderText(const std::string& text, float x, float y, float scale = 1.0f);
        void renderRaceInfo();
        void renderRaceStats();
        std::string formatTime(float timeInSeconds);
        
    public:
        void enter(Application* app, RaceSystem* raceSystem);
        void update(World* world, float deltaTime);
        void render();
        
        void toggleHUD() { showHUD = !showHUD; }
        bool isHUDVisible() const { return showHUD; }
    };

}
