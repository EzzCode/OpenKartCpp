#include "hud-system.hpp"
#include "../ecs/world.hpp"
#include "../components/race.hpp"
#include "../components/text-renderer.hpp"
#include <algorithm>
#include <sstream>
#include <iomanip>
#include <iostream>

namespace our
{

    void HUDSystem::enter(Application *app, RaceSystem *raceSystem, TextRenderSystem *textRenderSystem, World *world)
    {
        this->app = app;
        this->raceSystem = raceSystem;
        this->textRenderSystem = textRenderSystem;
        this->world = world;

        // Initialize HUD text entities
        auto windowSize = app->getFrameBufferSize();
        float centerX = windowSize.x / 2.0f;
        float topY = windowSize.y - 50.0f;
        float leftX = 20.0f;
        float rightX = windowSize.x - 200.0f;

        // Create HUD text entities with error handling
        try {
            raceStateEntity = createTextEntity("", centerX, topY, 1.0f, glm::vec3(1.0f, 1.0f, 0.0f), true);
            raceTimeEntity = createTextEntity("", leftX, topY - 50.0f, 1.0f, glm::vec3(1.0f, 1.0f, 1.0f));
            playerInfoEntity = createTextEntity("", leftX, topY - 90.0f, 0.8f, glm::vec3(1.0f, 1.0f, 1.0f));
            controlsHintEntity = createTextEntity("ESC: Menu | Arrow Keys: Drive", leftX, 30.0f, 0.6f, glm::vec3(0.6f, 0.6f, 0.6f));
            speedLabelEntity = createTextEntity("SPEED", rightX, 140.0f, 0.8f, glm::vec3(0.7f, 0.7f, 0.7f));
            speedEntity = createTextEntity("", rightX, 100.0f, 1.2f, glm::vec3(1.0f, 1.0f, 1.0f));
            countdownEntity = createTextEntity("", centerX, windowSize.y / 2.0f + 50.0f, 2.0f, glm::vec3(1.0f, 0.3f, 0.3f), true);
            titleEntity = createTextEntity("", centerX, topY - 50.0f, 1.5f, glm::vec3(1.0f, 1.0f, 1.0f), true);
        } catch (const std::exception& e) {
            std::cerr << "Error creating HUD entities: " << e.what() << std::endl;
        }
    }

    void HUDSystem::update(World *world, float deltaTime)
    {
        if (!world || !showHUD || !raceSystem) return;
        
        animationTime += deltaTime;
        updateHUDElements();
    }

    void HUDSystem::render()
    {
        if (!showHUD || !textRenderSystem || !world)
            return;

        try {
            textRenderSystem->render(world);
        } catch (const std::exception& e) {
            std::cerr << "Error rendering HUD: " << e.what() << std::endl;
        }
    }

    Entity *HUDSystem::createTextEntity(const std::string &text, float x, float y, float scale,
                                        const glm::vec3 &color, bool centered)
    {
        if (!world) return nullptr;

        Entity *entity = world->add();
        if (!entity) return nullptr;

        entity->localTransform.position = glm::vec3(x, y, 0.0f);
        entity->localTransform.scale = glm::vec3(scale, scale, 1.0f);

        auto textRenderer = entity->addComponent<TextRendererComponent>();
        if (!textRenderer) {
            // Use safer cleanup - don't call markForRemoval if it doesn't exist
            return nullptr;
        }

        textRenderer->text = text;
        textRenderer->color = color;
        textRenderer->scale = scale;
        textRenderer->centered = centered;
        textRenderer->visible = true;

        return entity;
    }

    void HUDSystem::updateTextEntity(Entity *entity, const std::string &text, const glm::vec3 &color)
    {
        if (!entity) return;

        auto textRenderer = entity->getComponent<TextRendererComponent>();
        if (textRenderer)
        {
            textRenderer->text = text;
            textRenderer->color = color;
        }
    }

    void HUDSystem::updateHUDElements()
    {
        if (!raceSystem || !app) return;

        auto windowSize = app->getFrameBufferSize();
        float centerX = windowSize.x / 2.0f;

        // Update race state with safe string handling
        std::string stateText = raceSystem->getRaceStateString();
        glm::vec3 stateColor = glm::vec3(1.0f, 1.0f, 0.0f);

        if (stateText.find("Ready") != std::string::npos) {
            stateColor = glm::vec3(0.0f, 1.0f, 0.0f);
        } else if (stateText.find("Racing") != std::string::npos) {
            stateColor = glm::vec3(1.0f, 0.5f, 0.0f);
        } else if (stateText.find("Completed") != std::string::npos) {
            stateColor = glm::vec3(0.0f, 1.0f, 1.0f);
        }

        updateTextEntity(raceStateEntity, stateText, stateColor);

        // During racing, show race information
        if (raceSystem->isRaceActive() || raceSystem->isRaceCompleted())
        {
            // Update race time
            std::stringstream timeStream;
            timeStream << std::fixed << std::setprecision(2) << raceSystem->getRaceTime();
            updateTextEntity(raceTimeEntity, "Race Time: " + timeStream.str() + "s", glm::vec3(1.0f, 1.0f, 1.0f));

            // Update player information with color coding
            std::stringstream playerInfo;
            playerInfo << "Lap: " << raceSystem->getCurrentLap();
            playerInfo << " | Position: " << raceSystem->getPosition();

            if (raceSystem->getBestLapTime() > 0.0f)
            {
                playerInfo << " | Best Lap: " << std::fixed << std::setprecision(2)
                           << raceSystem->getBestLapTime() << "s";
            }

            // Color-code position
            glm::vec3 positionColor = glm::vec3(1.0f, 1.0f, 1.0f); // White default
            int position = raceSystem->getPosition();
            if (position == 1)
            {
                positionColor = glm::vec3(0.0f, 1.0f, 0.0f); // Green for 1st
            }
            else if (position == 2)
            {
                positionColor = glm::vec3(1.0f, 1.0f, 0.0f); // Yellow for 2nd
            }
            else if (position == 3)
            {
                positionColor = glm::vec3(1.0f, 0.5f, 0.0f); // Orange for 3rd
            }

            updateTextEntity(playerInfoEntity, playerInfo.str(), positionColor);

            // Show player info and race time
            if (auto textRenderer = playerInfoEntity->getComponent<TextRendererComponent>())
            {
                textRenderer->visible = true;
            }
            if (auto textRenderer = raceTimeEntity->getComponent<TextRendererComponent>())
            {
                textRenderer->visible = true;
            }
        }
        else
        {
            // Hide race stats when not racing
            if (auto textRenderer = playerInfoEntity->getComponent<TextRendererComponent>())
            {
                textRenderer->visible = false;
            }
            if (auto textRenderer = raceTimeEntity->getComponent<TextRendererComponent>())
            {
                textRenderer->visible = false;
            }
        }

        // Update speedometer
        if (raceSystem->isRaceActive())
        {
            float currentSpeed = 0.0f; // Mock speed - replace with actual vehicle speed if available
            float maxSpeed = 100.0f;

            glm::vec3 speedColor = getSpeedColor(currentSpeed, maxSpeed);

            std::stringstream speedStream;
            speedStream << std::fixed << std::setprecision(1) << currentSpeed << " km/h";

            updateTextEntity(speedEntity, speedStream.str(), speedColor);

            // Show speedometer
            if (auto textRenderer = speedLabelEntity->getComponent<TextRendererComponent>())
            {
                textRenderer->visible = true;
            }
            if (auto textRenderer = speedEntity->getComponent<TextRendererComponent>())
            {
                textRenderer->visible = true;
            }
        }
        else
        {
            // Hide speedometer when not racing
            if (auto textRenderer = speedLabelEntity->getComponent<TextRendererComponent>())
            {
                textRenderer->visible = false;
            }
            if (auto textRenderer = speedEntity->getComponent<TextRendererComponent>())
            {
                textRenderer->visible = false;
            }
        }

        // Show countdown timer if race hasn't started
        if (!raceSystem->isRaceActive() && !raceSystem->isRaceCompleted())
        {
            // Animated countdown effect with pulsing
            glm::vec3 countdownColor = getPulsingColor(animationTime * 2.0f, glm::vec3(1.0f, 0.3f, 0.3f));
            float scale = 1.0f + 0.5f * sin(animationTime * 4.0f);

            updateTextEntity(countdownEntity, "GET READY!", countdownColor);

            // Update scale through the entity's transform
            if (countdownEntity)
            {
                countdownEntity->localTransform.scale = glm::vec3(scale, scale, 1.0f);
                if (auto textRenderer = countdownEntity->getComponent<TextRendererComponent>())
                {
                    textRenderer->visible = true;
                }
            }
        }
        else
        {
            // Hide countdown
            if (auto textRenderer = countdownEntity->getComponent<TextRendererComponent>())
            {
                textRenderer->visible = false;
            }
        }

        // Show animated title during race preparation
        if (stateText.find("Ready") != std::string::npos)
        {
            // Rainbow colored animated title
            glm::vec3 titleColor = glm::vec3(
                0.5f + 0.5f * sin(animationTime * 2.0f),
                0.5f + 0.5f * sin(animationTime * 2.0f + 2.094f),
                0.5f + 0.5f * sin(animationTime * 2.0f + 4.188f));

            float titleScale = 0.5f + 0.2f * sin(animationTime * 3.0f);

            updateTextEntity(titleEntity, "RACING CHAMPIONSHIP", titleColor);

            // Update scale and position
            if (titleEntity)
            {
                titleEntity->localTransform.scale = glm::vec3(titleScale, titleScale, 1.0f);
                titleEntity->localTransform.position.x = centerX;
                if (auto textRenderer = titleEntity->getComponent<TextRendererComponent>())
                {
                    textRenderer->visible = true;
                }
            }
        }
        else
        {
            // Hide title
            if (auto textRenderer = titleEntity->getComponent<TextRendererComponent>())
            {
                textRenderer->visible = false;
            }
        }
    }

    glm::vec3 HUDSystem::getSpeedColor(float speed, float maxSpeed)
    {
        float ratio = speed / maxSpeed;
        if (ratio < 0.3f)
        {
            return glm::vec3(0.0f, 1.0f, 0.0f); // Green for low speed
        }
        else if (ratio < 0.7f)
        {
            return glm::vec3(1.0f, 1.0f, 0.0f); // Yellow for medium speed
        }
        else
        {
            return glm::vec3(1.0f, 0.0f, 0.0f); // Red for high speed
        }
    }

    glm::vec3 HUDSystem::getPulsingColor(float time, const glm::vec3 &baseColor)
    {
        float pulse = 0.5f + 0.5f * sin(time);
        return baseColor * pulse + glm::vec3(0.3f) * (1.0f - pulse);
    }

    std::string HUDSystem::formatTime(float timeInSeconds)
    {
        int minutes = (int)(timeInSeconds / 60.0f);
        float seconds = timeInSeconds - (minutes * 60.0f);

        std::stringstream stream;
        if (minutes > 0)
        {
            stream << minutes << ":";
            stream << std::fixed << std::setprecision(2) << std::setw(5) << std::setfill('0') << seconds;
        }
        else
        {
            stream << std::fixed << std::setprecision(2) << seconds << "s";
        }

        return stream.str();
    }

}
