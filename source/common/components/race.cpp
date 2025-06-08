#include "race.hpp"

namespace our {    void CheckpointComponent::deserialize(const nlohmann::json& data) {
        checkpointIndex = data.value("checkpointIndex", 0);
        radius = data.value("radius", 5.0f);
        isFinishLine = data.value("isFinishLine", false);
    }

    void RacePlayerComponent::deserialize(const nlohmann::json& data) {
        maxLaps = data.value("maxLaps", 3);
        currentLap = data.value("currentLap", 1);
        nextCheckpoint = data.value("nextCheckpoint", 0);
        position = data.value("position", 1);
    }

    void RaceManagerComponent::deserialize(const nlohmann::json& data) {
        countdownTime = data.value("countdownTime", 3.0f);
        totalCheckpoints = data.value("totalCheckpoints", 0);
    }

}
