#pragma once

#include "../ecs/world.hpp"
#include "../application.hpp"
#include "race-system.hpp"
#include "text-render-system.hpp"
#include "../components/text-renderer.hpp"
#include <string>
#include <map>

namespace our
{

    class HUDSystem
    {
    private:
        Application *app;
        RaceSystem *raceSystem;
        TextRenderSystem *textRenderSystem;
        World *world;

        // Display variables
        bool showHUD = true;
        float fontSize = 24.0f;
        float animationTime = 0.0f; // For text animations

        // HUD Text entities - managed by this system
        Entity *raceStateEntity = nullptr;
        Entity *raceTimeEntity = nullptr;
        Entity *playerInfoEntity = nullptr;
        Entity *controlsHintEntity = nullptr;
        Entity *speedEntity = nullptr;
        Entity *speedLabelEntity = nullptr;
        Entity *countdownEntity = nullptr;
        Entity *titleEntity = nullptr;

        // Helper functions for managing HUD entities
        Entity *createTextEntity(const std::string &text, float x, float y, float scale = 1.0f,
                                 const glm::vec3 &color = glm::vec3(1.0f), bool centered = false);
        void updateTextEntity(Entity *entity, const std::string &text, const glm::vec3 &color);
        void updateHUDElements();
        std::string formatTime(float timeInSeconds);

        // Color utilities
        glm::vec3 getSpeedColor(float speed, float maxSpeed);
        glm::vec3 getPulsingColor(float time, const glm::vec3 &baseColor);

    public:
        void enter(Application *app, RaceSystem *raceSystem, TextRenderSystem *textRenderSystem, World *world);
        void update(World *world, float deltaTime);
        void render();

        void toggleHUD() { showHUD = !showHUD; }
        bool isHUDVisible() const { return showHUD; }
    };

}
