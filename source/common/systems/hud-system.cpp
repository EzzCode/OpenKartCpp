#include "hud-system.hpp"
#include "../ecs/world.hpp"
#include "../components/race.hpp"
#include <algorithm>
#include <sstream>
#include <iomanip>

namespace our {

    void HUDSystem::enter(Application* app, RaceSystem* raceSystem) {
        this->app = app;
        this->raceSystem = raceSystem;
    }

    void HUDSystem::update(World* world, float deltaTime) {
        // Nothing to update for HUD
    }

    void HUDSystem::render() {
        if (!showHUD || !raceSystem) return;

        renderRaceInfo();
    }

    void HUDSystem::renderText(const std::string& text, float x, float y, float scale) {
        // For now, we'll use a simple debug output since the engine doesn't have a text rendering system
        // In a real implementation, this would render text using bitmap fonts or a text rendering library
        
        // This is a placeholder - in a full implementation you would:
        // 1. Load a font texture atlas
        // 2. Create text geometry from the string
        // 3. Render using textured quads
        
        // For debugging purposes, we'll just store the text info
        // The actual rendering would need a proper text rendering system
    }    void HUDSystem::renderRaceInfo() {
        if (!raceSystem) return;

        // Get window size for positioning
        auto windowSize = app->getFrameBufferSize();
        float centerX = windowSize.x / 2.0f;
        float topY = windowSize.y - 50.0f;

        // Display race state
        std::string stateText = raceSystem->getRaceStateString();

        // Render state text (placeholder)
        renderText(stateText, centerX - 100, topY, 1.5f);

        // During racing, show race information
        if (raceSystem->isRaceActive() || raceSystem->isRaceCompleted()) {
            renderRaceStats();
        }
    }    void HUDSystem::renderRaceStats() {
        if (!raceSystem) return;

        auto windowSize = app->getFrameBufferSize();
        float leftX = 20.0f;
        float topY = windowSize.y - 100.0f;

        // Display race time
        std::stringstream timeStream;
        timeStream << std::fixed << std::setprecision(2) << raceSystem->getRaceTime();
        renderText("Race Time: " + timeStream.str() + "s", leftX, topY, 1.0f);

        // Display current player information
        float yOffset = topY - 40.0f;
        
        std::stringstream playerInfo;
        playerInfo << "Lap: " << raceSystem->getCurrentLap();        playerInfo << " | Position: " << raceSystem->getPosition();
        
        if (raceSystem->getBestLapTime() > 0.0f) {
            playerInfo << " | Best Lap: " << std::fixed << std::setprecision(2) 
                      << raceSystem->getBestLapTime() << "s";
        }
          renderText(playerInfo.str(), leftX, yOffset, 0.8f);
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
