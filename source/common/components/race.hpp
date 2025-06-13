#pragma once

#include "../ecs/component.hpp"
#include <glm/glm.hpp>
#include <vector>

namespace our
{ // Component to mark checkpoints in the race track
    class CheckpointComponent : public Component
    {
    public:
        int checkpointIndex = 0;   // Order of the checkpoint (0 = start/finish line)
        float radius = 5.0f;       // Detection radius for the checkpoint
        bool isFinishLine = false; // True if this is the finish line
        bool isVisible = false;    // Controls whether this checkpoint is visible

        virtual ~CheckpointComponent() = default;

        static std::string getID() { return "checkpoint"; }

        void deserialize(const nlohmann::json &data) override;
    }; // Component to track player's race progress
    class RacePlayerComponent : public Component
    {
    public:
        int currentLap = 1;
        int maxLaps = 3;
        int nextCheckpoint = 0; // Index of next checkpoint to reach
        float raceTime = 0.0f;  // Total race time
        float bestLapTime = 0.0f;
        float currentLapTime = 0.0f;
        bool raceCompleted = false;
        int position = 1; // Current race position

        std::vector<float> lapTimes; // Store lap times

        virtual ~RacePlayerComponent() = default;

        static std::string getID() { return "race-player"; }

        void deserialize(const nlohmann::json &data) override;
    };

    // Component for race management and timing
    class RaceManagerComponent : public Component
    {
    public:
        enum class RaceState
        {
            WAITING,
            COUNTDOWN,
            RACING,
            FINISHED
        };

        RaceState state = RaceState::WAITING;
        float countdownTime = 3.0f;
        float raceStartTime = 0.0f;
        int totalCheckpoints = 0;
        bool raceStarted = false;

        virtual ~RaceManagerComponent() = default;

        static std::string getID() { return "race-manager"; }

        void deserialize(const nlohmann::json &data) override;
    };

}
