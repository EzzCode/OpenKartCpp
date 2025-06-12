#pragma once

#include "../ecs/world.hpp"
#include "../application.hpp"
#include "race-system.hpp"
#include "forward-renderer.hpp"
#include <string>

namespace our
{

    class HUDSystem
    {
    private:
        Application *app;
        RaceSystem *raceSystem;
        ForwardRenderer *renderer;

        // Display variables
        bool showHUD = true;
        float fontSize = 24.0f;
        float animationTime = 0.0f; // For text animations

        // Helper functions for text rendering
        void renderText(const std::string &text, float x, float y, float scale = 1.0f, const glm::vec3 &color = glm::vec3(1.0f));
        void renderRaceInfo();
        void renderRaceStats();
        void renderSpeedometer();
        void renderCountdownTimer();
        std::string formatTime(float timeInSeconds);

        // Color utilities
        glm::vec3 getSpeedColor(float speed, float maxSpeed);
        glm::vec3 getPulsingColor(float time, const glm::vec3 &baseColor);

    public:
        void enter(Application *app, RaceSystem *raceSystem, ForwardRenderer *renderer);
        void update(World *world, float deltaTime);
        void render();

        void toggleHUD() { showHUD = !showHUD; }
        bool isHUDVisible() const { return showHUD; }
    };

}
