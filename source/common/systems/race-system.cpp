#include "race-system.hpp"
#include "../components/movement.hpp"
#include "../components/rigidbody.hpp"
#include <iostream>
#include <GLFW/glfw3.h>

namespace our
{
    void RaceSystem::enter(Application *app)
    {
        this->app = app;
        this->raceManager = nullptr;
        std::cout << "Race System: Initialized" << std::endl;
    }

    void RaceSystem::update(World *world, float deltaTime)
    {
        this->deltaTime = deltaTime;
        // Find race manager
        if (!raceManager)
        {
            for (auto entity : world->getEntities())
            {
                if (entity->getComponent<RaceManagerComponent>())
                {
                    raceManager = entity;
                    std::cout << "Race System: Found race manager" << std::endl;
                    break;
                }
            }
        }

        if (!raceManager)
            return;

        auto manager = raceManager->getComponent<RaceManagerComponent>();
        if (!manager)
            return;
        // Collect players and checkpoints
        players.clear();
        checkpoints.clear();

        for (auto entity : world->getEntities())
        {
            if (entity->getComponent<RacePlayerComponent>())
            {
                players.push_back(entity);
            }
            if (entity->getComponent<CheckpointComponent>())
            {
                checkpoints.push_back(entity);
            }
        } // Debug output for first frame and initialization
        if (!systemInitialized)
        {
            std::cout << "Race System: Found " << players.size() << " players and " << checkpoints.size() << " checkpoints" << std::endl;

            // Debug checkpoint positions after sorting
            for (size_t i = 0; i < checkpoints.size(); ++i)
            {
                auto checkpoint = checkpoints[i]->getComponent<CheckpointComponent>();
                auto pos = checkpoints[i]->localTransform.position;
                std::cout << "Checkpoint " << checkpoint->checkpointIndex
                          << " at position (" << pos.x << ", " << pos.y << ", " << pos.z << ")"
                          << " isFinishLine: " << checkpoint->isFinishLine << std::endl;
            }

            systemInitialized = true;
            initializationTime = glfwGetTime();
            std::cout << "Race System: Waiting 3 seconds before allowing race start. Press R to start manually." << std::endl;
        }

        // Auto-start race after 3 seconds, or allow manual start with R key
        if (manager->state == RaceManagerComponent::RaceState::WAITING &&
            glfwGetTime() - initializationTime > 2.0f)
        {
            std::cout << "Race System: Auto-starting race after initialization delay." << std::endl;
            startRace();
        }

        // Sort checkpoints by index
        std::sort(checkpoints.begin(), checkpoints.end(),
                  [](Entity *a, Entity *b)
                  {
                      auto checkA = a->getComponent<CheckpointComponent>();
                      auto checkB = b->getComponent<CheckpointComponent>();
                      return checkA->checkpointIndex < checkB->checkpointIndex;
                  });

        manager->totalCheckpoints = checkpoints.size();

        // Update checkpoint visibility
        updateCheckpointVisibility();

        // Handle race states
        switch (manager->state)
        {
        case RaceManagerComponent::RaceState::WAITING:
            // Start countdown when R key is pressed
            if (app->getKeyboard().justPressed(GLFW_KEY_R))
            {
                manager->state = RaceManagerComponent::RaceState::COUNTDOWN;
                std::cout << "Race starting in 3..." << std::endl;
            }
            break;

        case RaceManagerComponent::RaceState::COUNTDOWN:
            updateCountdown(manager, deltaTime);
            break;
        case RaceManagerComponent::RaceState::RACING:
        {
            // Update race time for all players
            for (auto player : players)
            {
                auto racePlayer = player->getComponent<RacePlayerComponent>();
                if (racePlayer && !racePlayer->raceCompleted)
                {
                    racePlayer->raceTime += deltaTime;
                    racePlayer->currentLapTime += deltaTime;
                    checkPlayerProgress(player, racePlayer);
                }
            }

            updateRacePositions();
            // Check if race is finished
            bool allFinished = true;
            int finishedPlayers = 0;
            for (auto player : players)
            {
                auto racePlayer = player->getComponent<RacePlayerComponent>();
                if (racePlayer && racePlayer->raceCompleted)
                {
                    finishedPlayers++;
                }
                else if (racePlayer)
                {
                    allFinished = false;
                }
            }

            if (allFinished && finishedPlayers > 0)
            {
                manager->state = RaceManagerComponent::RaceState::FINISHED;
                std::cout << "*** ALL PLAYERS FINISHED THE RACE! ***" << std::endl;
                std::cout << "Final Results:" << std::endl;
                for (size_t i = 0; i < players.size(); ++i)
                {
                    auto racePlayer = players[i]->getComponent<RacePlayerComponent>();
                    if (racePlayer)
                    {
                        std::cout << "Position " << (i + 1) << ": Total time "
                                  << racePlayer->raceTime << "s, Best lap: "
                                  << racePlayer->bestLapTime << "s" << std::endl;
                    }
                }
                std::cout << "Press R to restart the race!" << std::endl;
            }
            break;
        }

        case RaceManagerComponent::RaceState::FINISHED:
            // Allow race restart
            if (app->getKeyboard().justPressed(GLFW_KEY_R))
            {
                resetRace();
            }
            break;
        }
    }
    void RaceSystem::updateCountdown(RaceManagerComponent *manager, float dt)
    {
        manager->countdownTime -= dt;

        int currentCountdown = (int)ceil(manager->countdownTime);

        if (currentCountdown != lastCountdownValue && currentCountdown > 0)
        {
            std::cout << currentCountdown << "..." << std::endl;
            lastCountdownValue = currentCountdown;
        }

        if (manager->countdownTime <= 0.0f)
        {
            manager->state = RaceManagerComponent::RaceState::RACING;
            manager->raceStartTime = glfwGetTime();
            manager->raceStarted = true;
            std::cout << "GO!" << std::endl;
            lastCountdownValue = 4; // Reset for next countdown
        }
    }
    void RaceSystem::checkPlayerProgress(Entity *player, RacePlayerComponent *racePlayer)
    {
        if (racePlayer->nextCheckpoint >= static_cast<int>(checkpoints.size()))
            return;

        Entity *nextCheckpointEntity = checkpoints[racePlayer->nextCheckpoint];
        auto checkpoint = nextCheckpointEntity->getComponent<CheckpointComponent>();

        // Calculate distance to checkpoint
        glm::vec3 playerPos = player->localTransform.position;
        glm::vec3 checkpointPos = nextCheckpointEntity->localTransform.position;
        float distance = glm::length(playerPos - checkpointPos);

        // Debug output every few seconds to track player progress
        static float lastDebugTime = 0;
        float currentTime = glfwGetTime();
        if (currentTime - lastDebugTime > 2.0f)
        {
            std::cout << "Player pos: (" << playerPos.x << ", " << playerPos.y << ", " << playerPos.z
                      << ") | Next checkpoint " << checkpoint->checkpointIndex
                      << " pos: (" << checkpointPos.x << ", " << checkpointPos.y << ", " << checkpointPos.z
                      << ") | Distance: " << distance << " (radius: " << checkpoint->radius << ")" << std::endl;
            lastDebugTime = currentTime;
        }
        // Check if player reached checkpoint
        if (distance <= checkpoint->radius)
        {
            std::cout << "*** CHECKPOINT " << checkpoint->checkpointIndex << " REACHED! ***" << std::endl;
            // Move to next checkpoint
            racePlayer->nextCheckpoint++;
            // Check if this was the finish line AND player has completed a full circuit
            // (nextCheckpoint will be 1 after incrementing from 0, meaning player just hit checkpoint 0)
            if (checkpoint->isFinishLine && racePlayer->nextCheckpoint == 1)
            {
                // Only complete lap if this isn't the very first checkpoint hit of the race
                if (racePlayer->raceTime > 1.0f)
                { // Prevent immediate lap completion at race start
                    // Record lap time
                    racePlayer->lapTimes.push_back(racePlayer->currentLapTime);

                    if (racePlayer->bestLapTime == 0.0f || racePlayer->currentLapTime < racePlayer->bestLapTime)
                    {
                        racePlayer->bestLapTime = racePlayer->currentLapTime;
                    }

                    std::cout << "Lap " << racePlayer->currentLap << " completed! Time: "
                              << racePlayer->currentLapTime << "s" << std::endl;

                    racePlayer->currentLapTime = 0.0f;
                    racePlayer->currentLap++;

                    // Check if race is complete
                    if (racePlayer->currentLap > racePlayer->maxLaps)
                    {
                        racePlayer->raceCompleted = true;
                        std::cout << "*** RACE COMPLETE! ***" << std::endl;
                        std::cout << "Total race time: " << racePlayer->raceTime << "s" << std::endl;
                        std::cout << "Best lap time: " << racePlayer->bestLapTime << "s" << std::endl;
                        std::cout << "Lap times: ";
                        for (size_t i = 0; i < racePlayer->lapTimes.size(); ++i)
                        {
                            std::cout << "Lap " << (i + 1) << ": " << racePlayer->lapTimes[i] << "s";
                            if (i < racePlayer->lapTimes.size() - 1)
                                std::cout << ", ";
                        }
                        std::cout << std::endl;
                    }
                    else
                    {
                        // Reset to checkpoint 1 for next lap (skip the start/finish line)
                        racePlayer->nextCheckpoint = 1;
                    }
                }
            }
            else
            {
                // Normal checkpoint progression
                // Wrap around checkpoint index if we've reached the end
                if (racePlayer->nextCheckpoint >= static_cast<int>(checkpoints.size()))
                {
                    racePlayer->nextCheckpoint = 0;
                }
            }
        }
    }

    void RaceSystem::updateRacePositions()
    {
        if (players.empty())
            return;

        // Sort players by race progress (lap + checkpoint progress)
        std::sort(players.begin(), players.end(),
                  [this](Entity *a, Entity *b)
                  {
                      auto playerA = a->getComponent<RacePlayerComponent>();
                      auto playerB = b->getComponent<RacePlayerComponent>();

                      if (!playerA || !playerB)
                          return false;

                      // Compare by lap first
                      if (playerA->currentLap != playerB->currentLap)
                      {
                          return playerA->currentLap > playerB->currentLap;
                      }

                      // Then by checkpoint progress
                      if (playerA->nextCheckpoint != playerB->nextCheckpoint)
                      {
                          return playerA->nextCheckpoint > playerB->nextCheckpoint;
                      }

                      // Finally by distance to next checkpoint (closer is better)
                      float distA = calculateDistanceToNextCheckpoint(a, playerA);
                      float distB = calculateDistanceToNextCheckpoint(b, playerB);
                      return distA < distB;
                  });

        // Update positions
        for (size_t i = 0; i < players.size(); ++i)
        {
            auto racePlayer = players[i]->getComponent<RacePlayerComponent>();
            if (racePlayer)
            {
                racePlayer->position = i + 1;
            }
        }
    }
    void RaceSystem::updateCheckpointVisibility()
    {
        // Hide all checkpoints first
        for (auto checkpoint : checkpoints)
        {
            auto checkpointComponent = checkpoint->getComponent<CheckpointComponent>();
            if (checkpointComponent)
            {
                checkpointComponent->isVisible = false;
            }
        }

        // Show only the next checkpoint for each player
        for (auto player : players)
        {
            auto racePlayer = player->getComponent<RacePlayerComponent>();
            if (racePlayer && racePlayer->nextCheckpoint < static_cast<int>(checkpoints.size()))
            {
                auto checkpointComponent = checkpoints[racePlayer->nextCheckpoint]->getComponent<CheckpointComponent>();
                if (checkpointComponent)
                {
                    checkpointComponent->isVisible = true;
                }
            }
        }
    }

    float RaceSystem::calculateDistanceToNextCheckpoint(Entity *player, RacePlayerComponent *racePlayer)
    {
        if (racePlayer->nextCheckpoint >= static_cast<int>(checkpoints.size()))
            return 0.0f;

        glm::vec3 playerPos = player->localTransform.position;
        glm::vec3 checkpointPos = checkpoints[racePlayer->nextCheckpoint]->localTransform.position;
        return glm::length(playerPos - checkpointPos);
    }
    void RaceSystem::startRace()
    {
        if (raceManager)
        {
            auto manager = raceManager->getComponent<RaceManagerComponent>();
            if (manager)
            {
                manager->state = RaceManagerComponent::RaceState::COUNTDOWN;
                manager->countdownTime = 3.0f; // Reset countdown time when starting
                lastCountdownValue = 4;        // Reset countdown display state
            }
        }
    }
    void RaceSystem::resetRace()
    {
        if (raceManager)
        {
            auto manager = raceManager->getComponent<RaceManagerComponent>();
            if (manager)
            {
                manager->state = RaceManagerComponent::RaceState::WAITING;
                manager->raceStarted = false;
                manager->countdownTime = 3.0f; // Reset countdown timer
            }
        }
        // Reset initialization timer for proper restart behavior
        initializationTime = glfwGetTime();
        lastCountdownValue = 4; // Reset countdown display state

        // Reset all players
        for (auto player : players)
        {
            auto racePlayer = player->getComponent<RacePlayerComponent>();
            if (racePlayer)
            {
                racePlayer->currentLap = 1;
                racePlayer->nextCheckpoint = 1; // Start targeting checkpoint 1, not 0
                racePlayer->raceTime = 0.0f;
                racePlayer->currentLapTime = 0.0f;
                racePlayer->raceCompleted = false;
                racePlayer->position = 1;
                racePlayer->lapTimes.clear();
            }
            // Reset player position to initial spawn location
            auto rigidbody = player->getComponent<RigidbodyComponent>();
            if (rigidbody)
            {
                // Use the initial spawn position stored during deserialization
                glm::vec3 spawnPosition = rigidbody->initialPosition;
                glm::vec3 spawnRotation = rigidbody->initialRotation;

                // Reset entity transform to initial position
                player->localTransform.position = spawnPosition;
                player->localTransform.rotation = spawnRotation;

                // Reset physics body if it exists
                if (rigidbody->rigidbody)
                {
                    // Convert to Bullet's coordinate system
                    btVector3 initialPos(spawnPosition.x, spawnPosition.y, spawnPosition.z);
                    // Create new transform with initial position and rotation
                    btTransform resetTransform;
                    resetTransform.setRotation(btQuaternion(spawnRotation.x, spawnRotation.y, spawnRotation.z));
                    resetTransform.setOrigin(initialPos);

                    // Reset the rigidbody position and clear velocities
                    rigidbody->rigidbody->setWorldTransform(resetTransform);
                    rigidbody->rigidbody->setLinearVelocity(btVector3(0, 0, 0));
                    rigidbody->rigidbody->setAngularVelocity(btVector3(0, 0, 0));
                    rigidbody->rigidbody->clearForces();

                    // Also update the motion state
                    btMotionState *motionState = rigidbody->rigidbody->getMotionState();
                    if (motionState)
                    {
                        motionState->setWorldTransform(resetTransform);
                    }

                    // Update the rigidbody component's position to match the initial spawn
                    rigidbody->position = spawnPosition;
                    rigidbody->rotation = spawnRotation;

                    std::cout << "Reset player position to spawn coordinates (" << spawnPosition.x
                              << ", " << spawnPosition.y << ", " << spawnPosition.z << ")" << std::endl;
                }
            }
        }

        std::cout << "Race reset! Press R to start a new race." << std::endl;
    }

    bool RaceSystem::isRaceActive() const
    {
        if (!raceManager)
            return false;
        auto manager = raceManager->getComponent<RaceManagerComponent>();
        return manager && manager->state == RaceManagerComponent::RaceState::RACING;
    }

    bool RaceSystem::isRaceCompleted() const
    {
        if (!raceManager)
            return false;
        auto manager = raceManager->getComponent<RaceManagerComponent>();
        return manager && manager->state == RaceManagerComponent::RaceState::FINISHED;
    }

    float RaceSystem::getRaceTime() const
    {
        if (players.empty())
            return 0.0f;
        auto racePlayer = players[0]->getComponent<RacePlayerComponent>();
        return racePlayer ? racePlayer->raceTime : 0.0f;
    }

    int RaceSystem::getCurrentLap() const
    {
        if (players.empty())
            return 1;
        auto racePlayer = players[0]->getComponent<RacePlayerComponent>();
        return racePlayer ? racePlayer->currentLap : 1;
    }

    int RaceSystem::getPosition() const
    {
        if (players.empty())
            return 1;
        auto racePlayer = players[0]->getComponent<RacePlayerComponent>();
        return racePlayer ? racePlayer->position : 1;
    }

    float RaceSystem::getBestLapTime() const
    {
        if (players.empty())
            return 0.0f;
        auto racePlayer = players[0]->getComponent<RacePlayerComponent>();
        return racePlayer ? racePlayer->bestLapTime : 0.0f;
    }

    std::string RaceSystem::getRaceStateString() const
    {
        if (!raceManager)
            return "No Race Manager";
        auto manager = raceManager->getComponent<RaceManagerComponent>();
        if (!manager)
            return "No Race Manager";

        switch (manager->state)
        {
        case RaceManagerComponent::RaceState::WAITING:
            return "Press R to Start Race";
        case RaceManagerComponent::RaceState::COUNTDOWN:
            return "Get Ready... " + std::to_string((int)ceil(manager->countdownTime));
        case RaceManagerComponent::RaceState::RACING:
            return "Racing!";
        case RaceManagerComponent::RaceState::FINISHED:
            return "Race Finished! Press R to Restart";
        default:
            return "Unknown State";
        }
    }
    bool RaceSystem::isInputDisabled() const
    {
        if (!raceManager)
            return true; // Disable input if no race manager found
        auto manager = raceManager->getComponent<RaceManagerComponent>();
        if (!manager)
            return true; // Disable input if no race manager component found

        // Disable input when race is in WAITING, COUNTDOWN, or FINISHED state
        return manager->state == RaceManagerComponent::RaceState::WAITING ||
               manager->state == RaceManagerComponent::RaceState::COUNTDOWN ||
               manager->state == RaceManagerComponent::RaceState::FINISHED;
    }
    int RaceSystem::getSpeed() const
    {
        auto rigidbodyComponent = players[0]->getComponent<RigidbodyComponent>();
        if (rigidbodyComponent && rigidbodyComponent->rigidbody)
        {
            btVector3 velocity = rigidbodyComponent->rigidbody->getLinearVelocity();
            return static_cast<int>(velocity.length());
        }
        return 0; // No speed if no rigidbody or not moving
    }

}
