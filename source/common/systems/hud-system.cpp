#include "hud-system.hpp"
#include "../ecs/world.hpp"
#include "../components/race.hpp"
#include <algorithm>
#include <sstream>
#include <iomanip>

namespace our {

    void HUDSystem::enter(Application* app, RaceSystem* raceSystem, ForwardRenderer* renderer) {
        this->app = app;
        this->raceSystem = raceSystem;
        this->renderer = renderer;
    }

    void HUDSystem::update(World* world, float deltaTime) {
        // Update animation time for text effects
        animationTime += deltaTime;
    }

    void HUDSystem::render() {
        if (!showHUD || !raceSystem || !renderer) return;

        renderRaceInfo();
        renderSpeedometer();
        
        // Show countdown timer if race hasn't started
        if (!raceSystem->isRaceActive() && !raceSystem->isRaceCompleted()) {
            renderCountdownTimer();
        }
        
        // Show animated title during race preparation
        if (raceSystem->getRaceStateString().find("Ready") != std::string::npos) {
            renderAnimatedTitle();
        }
    }

    void HUDSystem::renderText(const std::string& text, float x, float y, float scale, const glm::vec3& color) {
        if (renderer) {
            renderer->renderText(text, x, y, scale, color);
        }
    }    void HUDSystem::renderRaceInfo() {
        if (!raceSystem) return;

        // Get window size for positioning
        auto windowSize = app->getFrameBufferSize();
        float centerX = windowSize.x / 2.0f;
        float topY = windowSize.y - 50.0f;

        // Display race state with different colors
        std::string stateText = raceSystem->getRaceStateString();
        glm::vec3 stateColor = glm::vec3(1.0f, 1.0f, 0.0f); // Yellow for general state
        
        // Color-code different race states
        if (stateText.find("Ready") != std::string::npos) {
            stateColor = glm::vec3(0.0f, 1.0f, 0.0f); // Green for ready
        } else if (stateText.find("Racing") != std::string::npos) {
            stateColor = glm::vec3(1.0f, 0.5f, 0.0f); // Orange for racing
        } else if (stateText.find("Completed") != std::string::npos) {
            stateColor = glm::vec3(0.0f, 1.0f, 1.0f); // Cyan for completed
        }

        // Render state text
        renderText(stateText, centerX - 100, topY, 1.5f, stateColor);

        // During racing, show race information
        if (raceSystem->isRaceActive() || raceSystem->isRaceCompleted()) {
            renderRaceStats();
        }
    }    void HUDSystem::renderRaceStats() {
        if (!raceSystem) return;

        auto windowSize = app->getFrameBufferSize();
        float leftX = 20.0f;
        float topY = windowSize.y - 100.0f;

        // Display race time with colored text
        std::stringstream timeStream;
        timeStream << std::fixed << std::setprecision(2) << raceSystem->getRaceTime();
        renderText("Race Time: " + timeStream.str() + "s", leftX, topY, 1.0f, glm::vec3(1.0f, 1.0f, 1.0f));

        // Display current player information with color coding
        float yOffset = topY - 40.0f;
        
        std::stringstream playerInfo;
        playerInfo << "Lap: " << raceSystem->getCurrentLap();
        playerInfo << " | Position: " << raceSystem->getPosition();
        
        if (raceSystem->getBestLapTime() > 0.0f) {
            playerInfo << " | Best Lap: " << std::fixed << std::setprecision(2) 
                      << raceSystem->getBestLapTime() << "s";
        }
        
        // Color-code position (green for 1st, yellow for 2nd, orange for 3rd, white for others)
        glm::vec3 positionColor = glm::vec3(1.0f, 1.0f, 1.0f); // White default
        int position = raceSystem->getPosition();
        if (position == 1) {
            positionColor = glm::vec3(0.0f, 1.0f, 0.0f); // Green for 1st
        } else if (position == 2) {
            positionColor = glm::vec3(1.0f, 1.0f, 0.0f); // Yellow for 2nd
        } else if (position == 3) {
            positionColor = glm::vec3(1.0f, 0.5f, 0.0f); // Orange for 3rd
        }
          renderText(playerInfo.str(), leftX, yOffset, 0.8f, positionColor);
        
        // Add controls hint
        yOffset = 30.0f; // Bottom of screen
        renderText("ESC: Menu | Arrow Keys: Drive", leftX, yOffset, 0.6f, glm::vec3(0.6f, 0.6f, 0.6f));
    }

    void HUDSystem::renderSpeedometer() {
        if (!raceSystem || !raceSystem->isRaceActive()) return;
        
        auto windowSize = app->getFrameBufferSize();
        float rightX = windowSize.x - 200.0f;
        float bottomY = 100.0f;
        
        // Mock speed calculation (you can replace with actual vehicle speed)
        float currentSpeed = 0.0f; // raceSystem->getCurrentSpeed() if available
        float maxSpeed = 100.0f;
        
        // Color-coded speed display
        glm::vec3 speedColor = getSpeedColor(currentSpeed, maxSpeed);
        
        std::stringstream speedStream;
        speedStream << std::fixed << std::setprecision(1) << currentSpeed << " km/h";
        
        renderText("SPEED", rightX, bottomY + 40.0f, 0.8f, glm::vec3(0.7f, 0.7f, 0.7f));
        renderText(speedStream.str(), rightX, bottomY, 1.2f, speedColor);
    }

    void HUDSystem::renderCountdownTimer() {
        auto windowSize = app->getFrameBufferSize();
        float centerX = windowSize.x / 2.0f;
        float centerY = windowSize.y / 2.0f;
        
        // Animated countdown effect
        glm::vec3 countdownColor = getPulsingColor(animationTime * 2.0f, glm::vec3(1.0f, 0.3f, 0.3f));
        float scale = 2.0f + 0.5f * sin(animationTime * 4.0f);
        
        renderText("GET READY!", centerX - 80.0f, centerY + 50.0f, scale, countdownColor);
    }

    void HUDSystem::renderAnimatedTitle() {
        auto windowSize = app->getFrameBufferSize();
        float centerX = windowSize.x / 2.0f;
        float topY = windowSize.y - 100.0f;
        
        // Rainbow colored animated title
        float rainbow = sin(animationTime * 3.0f);
        glm::vec3 titleColor = glm::vec3(
            0.5f + 0.5f * sin(animationTime * 2.0f),
            0.5f + 0.5f * sin(animationTime * 2.0f + 2.094f),
            0.5f + 0.5f * sin(animationTime * 2.0f + 4.188f)
        );
        
        float titleScale = 1.5f + 0.2f * sin(animationTime * 3.0f);
        renderText("RACING CHAMPIONSHIP", centerX - 150.0f, topY, titleScale, titleColor);
    }

    glm::vec3 HUDSystem::getSpeedColor(float speed, float maxSpeed) {
        float ratio = speed / maxSpeed;
        if (ratio < 0.3f) {
            return glm::vec3(0.0f, 1.0f, 0.0f); // Green for low speed
        } else if (ratio < 0.7f) {
            return glm::vec3(1.0f, 1.0f, 0.0f); // Yellow for medium speed
        } else {
            return glm::vec3(1.0f, 0.0f, 0.0f); // Red for high speed
        }
    }

    glm::vec3 HUDSystem::getPulsingColor(float time, const glm::vec3& baseColor) {
        float pulse = 0.5f + 0.5f * sin(time);
        return baseColor * pulse + glm::vec3(0.3f) * (1.0f - pulse);
    }

    std::string HUDSystem::formatTime(float timeInSeconds) {
        int minutes = (int)(timeInSeconds / 60.0f);
        float seconds = timeInSeconds - (minutes * 60.0f);
        
        std::stringstream stream;
        if (minutes > 0) {
            stream << minutes << ":";
            stream << std::fixed << std::setprecision(2) << std::setw(5) << std::setfill('0') << seconds;
        } else {
            stream << std::fixed << std::setprecision(2) << seconds << "s";
        }
        
        return stream.str();
    }

}
