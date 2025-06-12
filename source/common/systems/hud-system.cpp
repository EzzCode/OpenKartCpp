#include "hud-system.hpp"
#include "../ecs/world.hpp"
#include "../components/race.hpp"

#include <algorithm>
#include <sstream>
#include <iomanip>
#include <iostream>

namespace our {

    // ========== Public Methods ==========

    void HUDSystem::enter(Application* app, RaceSystem* raceSystem, ForwardRenderer* renderer) {
        this->app = app;
        this->raceSystem = raceSystem;
        this->renderer = renderer;
    }

    void HUDSystem::update(World* world, float deltaTime) {
        animationTime += deltaTime;
    }

    void HUDSystem::render() {
        if (!showHUD || !raceSystem || !renderer) return;

        renderRaceInfo();
        renderSpeedometer();

        if (!raceSystem->isRaceActive() && !raceSystem->isRaceCompleted()) {
            renderCountdownTimer();
        }

        if (raceSystem->getRaceStateString().find("Ready") != std::string::npos) {
            renderAnimatedTitle();
        }
    }

    // ========== Core Rendering Helpers ==========

    void HUDSystem::renderRaceInfo() {
        if (!raceSystem) return;

        auto windowSize = app->getFrameBufferSize();
        float centerX = windowSize.x / 2.0f;
        float topY = windowSize.y - 50.0f;

        std::string stateText = raceSystem->getRaceStateString();
        glm::vec3 stateColor = glm::vec3(1.0f, 1.0f, 0.0f); // Default yellow

        if (stateText.find("Ready") != std::string::npos)
            stateColor = glm::vec3(0.0f, 1.0f, 0.0f); // Green
        else if (stateText.find("Racing") != std::string::npos)
            stateColor = glm::vec3(1.0f, 0.5f, 0.0f); // Orange
        else if (stateText.find("Completed") != std::string::npos)
            stateColor = glm::vec3(0.0f, 1.0f, 1.0f); // Cyan

        renderer->renderTextCentered(stateText, centerX, topY, 1.5f, stateColor);

        if (raceSystem->isRaceActive() || raceSystem->isRaceCompleted()) {
            renderRaceStats();
        }
    }

    void HUDSystem::renderRaceStats() {
        if (!raceSystem) return;

        auto windowSize = app->getFrameBufferSize();
        float leftX = 20.0f;
        float topY = windowSize.y - 50.0f;
        float yOffset = topY - 90.0f;

        // Race Time
        std::stringstream timeStream;
        timeStream << std::fixed << std::setprecision(2) << raceSystem->getRaceTime();
        renderer->renderText("Race Time: " + timeStream.str() + "s", leftX, topY - 50.0f, 1.0f, glm::vec3(1.0f));

        // Lap, Position, Best Lap
        std::stringstream playerInfo;
        playerInfo << "Lap: " << raceSystem->getCurrentLap()
                   << " | Position: " << raceSystem->getPosition();

        if (raceSystem->getBestLapTime() > 0.0f) {
            playerInfo << " | Best Lap: " << std::fixed << std::setprecision(2)
                       << raceSystem->getBestLapTime() << "s";
        }

        glm::vec3 positionColor = glm::vec3(1.0f);
        int position = raceSystem->getPosition();

        if (position == 1) positionColor = glm::vec3(0.0f, 1.0f, 0.0f);      // Green
        else if (position == 2) positionColor = glm::vec3(1.0f, 1.0f, 0.0f); // Yellow
        else if (position == 3) positionColor = glm::vec3(1.0f, 0.5f, 0.0f); // Orange

        renderer->renderText(playerInfo.str(), leftX, yOffset, 0.8f, positionColor);

        // Controls hint
        renderer->renderText("ESC: Menu | Arrow Keys: Drive", leftX, 30.0f, 0.6f, glm::vec3(0.6f));
    }

    void HUDSystem::renderSpeedometer() {
        if (!raceSystem || !raceSystem->isRaceActive()) return;

        auto windowSize = app->getFrameBufferSize();
        float rightX = windowSize.x - 200.0f;
        float bottomY = 100.0f;

        float currentSpeed = 0.0f; // Replace with real speed if available
        float maxSpeed = 100.0f;

        glm::vec3 speedColor = getSpeedColor(currentSpeed, maxSpeed);

        std::stringstream speedStream;
        speedStream << std::fixed << std::setprecision(1) << currentSpeed << " km/h";

        renderer->renderText("SPEED", rightX, bottomY + 40.0f, 0.8f, glm::vec3(0.7f));
        renderer->renderText(speedStream.str(), rightX, bottomY, 1.2f, speedColor);
    }

    void HUDSystem::renderCountdownTimer() {
        auto windowSize = app->getFrameBufferSize();
        float centerX = windowSize.x / 2.0f;
        float centerY = windowSize.y / 2.0f;

        glm::vec3 countdownColor = getPulsingColor(animationTime * 2.0f, glm::vec3(1.0f, 0.3f, 0.3f));
        float scale = 1.5f + 0.5f * sin(animationTime * 4.0f);

        renderer->renderTextCentered("GET READY!", centerX, centerY + 50.0f, scale, countdownColor);
    }

    void HUDSystem::renderAnimatedTitle() {
        auto windowSize = app->getFrameBufferSize();
        float centerX = windowSize.x / 2.0f;
        float topY = windowSize.y - 50.0f;

        glm::vec3 titleColor = glm::vec3(
            0.5f + 0.5f * sin(animationTime * 2.0f),
            0.5f + 0.5f * sin(animationTime * 2.0f + 2.094f),
            0.5f + 0.5f * sin(animationTime * 2.0f + 4.188f)
        );

        float titleScale = 1.5f + 0.2f * sin(animationTime * 3.0f);
        renderer->renderTextCentered("RACING CHAMPIONSHIP", centerX, topY - 50.0f, titleScale, titleColor);
    }

    // ========== Utility Methods ==========

    glm::vec3 HUDSystem::getSpeedColor(float speed, float maxSpeed) {
        float ratio = speed / maxSpeed;
        if (ratio < 0.3f) return glm::vec3(0.0f, 1.0f, 0.0f);  // Green
        if (ratio < 0.7f) return glm::vec3(1.0f, 1.0f, 0.0f);  // Yellow
        return glm::vec3(1.0f, 0.0f, 0.0f);                    // Red
    }

    glm::vec3 HUDSystem::getPulsingColor(float time, const glm::vec3& baseColor) {
        float pulse = 0.5f + 0.5f * sin(time);
        return baseColor * pulse + glm::vec3(0.3f) * (1.0f - pulse);
    }

    std::string HUDSystem::formatTime(float timeInSeconds) {
        int minutes = static_cast<int>(timeInSeconds / 60.0f);
        float seconds = timeInSeconds - minutes * 60.0f;

        std::stringstream stream;
        if (minutes > 0) {
            stream << minutes << ":"
                   << std::fixed << std::setprecision(2) << std::setw(5) << std::setfill('0') << seconds;
        } else {
            stream << std::fixed << std::setprecision(2) << seconds << "s";
        }
        return stream.str();
    }

}
