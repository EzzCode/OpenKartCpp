#pragma once

#include "../ecs/world.hpp"
#include "../components/input.hpp"
#include "../application.hpp"
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>
#include <glm/trigonometric.hpp>
#include <glm/gtx/fast_trigonometry.hpp>
#include <btBulletDynamicsCommon.h>
#include "race-system.hpp"

namespace our
{
    class InputMovementSystem
    {
    private:
        Application *app;
        RaceSystem *raceSystem = nullptr; // Reference to race system for input checking

        // Vehicle properties
        float engineForce = 0.0f;
        float brakingForce = 0.0f;
        float steeringAngle = 0.0f;
        float currentSpeed = 0.0f;

        // Constants
        const float MAX_ENGINE_FORCE = 6000.0f;   // Increased for faster acceleration
        const float MAX_BRAKING_FORCE = 1500.0f;  // Reduced for less abrupt stopping
        const float STEERING_INCREMENT = 0.06f;   // Slightly more sensitive steering
        const float STEERING_CLAMP = 0.6f;        // Increased max steering angle
        const float VEHICLE_MASS = 20.0f;         // Reduced mass for lighter handling
        const float DRAG_COEFFICIENT = 0.3f;      // Reduced drag for less deceleration
        const float ACCELERATION_RATE = 3000.0f;  // Faster response to input
        const float DECELERATION_RATE = 4000.0f;  // Snappier slowdown when releasing throttle
        const float MAX_SPEED = 60.0f;            // Increased top speed
        const float REVERSE_SPEED_LIMIT = -15.0f; // Reduced reverse speed for balance

    public:
        void enter(Application *app)
        {
            this->app = app;
        }

        void setRaceSystem(RaceSystem *raceSystem)
        {
            this->raceSystem = raceSystem;
        }
        void update(World *world, float deltaTime)
        {
            for (auto entity : world->getEntities())
            {
                InputComponent *input = entity->getComponent<InputComponent>();
                if (!input)
                    continue;

                // Check if input should be disabled based on race state
                bool inputDisabled = raceSystem && raceSystem->isInputDisabled();
                if (inputDisabled)
                    continue; // Skip input processing if disabled

                // Reset forces
                brakingForce = 0.0f;

                // Forward/Backward movement
                if (app->getKeyboard().isPressed(GLFW_KEY_UP))
                {
                    // Accelerate forward
                    engineForce += ACCELERATION_RATE * deltaTime;
                    engineForce = std::min(engineForce, MAX_ENGINE_FORCE);
                }
                else if (app->getKeyboard().isPressed(GLFW_KEY_DOWN))
                {
                    if (currentSpeed > 0)
                    {
                        // Apply brakes if moving forward
                        brakingForce = MAX_BRAKING_FORCE;
                    }
                    else
                    {
                        // Reverse if stationary or moving backward
                        engineForce -= ACCELERATION_RATE * deltaTime;
                        engineForce = std::max(engineForce, -MAX_ENGINE_FORCE * 0.5f);
                    }
                }
                else
                {
                    // Natural deceleration when no input
                    if (abs(engineForce) > 0)
                    {
                        float decel = DECELERATION_RATE * deltaTime;
                        if (engineForce > 0)
                        {
                            engineForce = std::max(0.0f, engineForce - decel);
                        }
                        else
                        {
                            engineForce = std::min(0.0f, engineForce + decel);
                        }
                    }
                }

                // Dynamic steering based on speed
                float speedFactor = 1.0f - (abs(currentSpeed) / MAX_SPEED) * 0.5f;
                if (app->getKeyboard().isPressed(GLFW_KEY_LEFT))
                {
                    // Turn left
                    steeringAngle += STEERING_INCREMENT * speedFactor;
                    steeringAngle = std::min(steeringAngle, STEERING_CLAMP);
                }
                else if (app->getKeyboard().isPressed(GLFW_KEY_RIGHT))
                {
                    // Turn right
                    steeringAngle -= STEERING_INCREMENT * speedFactor;
                    steeringAngle = std::max(steeringAngle, -STEERING_CLAMP);
                }
                else
                {
                    // Gradual return to center
                    steeringAngle *= (1.0f - deltaTime * 3.0f); // Faster self-centering
                }

                // Calculate vehicle physics
                glm::vec3 forwardDir = glm::normalize(
                    glm::vec3(sin(entity->localTransform.rotation.y),
                              0,
                              cos(entity->localTransform.rotation.y)));

                // Update speed and apply forces
                currentSpeed += (engineForce / VEHICLE_MASS) * deltaTime;
                currentSpeed = glm::clamp(currentSpeed, REVERSE_SPEED_LIMIT, MAX_SPEED);

                // Apply drag (reduces effect at lower speeds for arcade-like feel)
                float dragForce = DRAG_COEFFICIENT * currentSpeed * currentSpeed * deltaTime;
                currentSpeed -= dragForce * glm::sign(currentSpeed);

                // Apply braking
                if (brakingForce > 0)
                {
                    currentSpeed *= (1.0f - brakingForce * deltaTime * 0.001f);
                }

                // Update position
                entity->localTransform.position += forwardDir * currentSpeed * deltaTime;

                // Steering effect based on speed
                float steeringEffect = steeringAngle * (1.0f + 0.3f * abs(currentSpeed / MAX_SPEED)) * deltaTime;
                entity->localTransform.rotation.y += steeringEffect;
            }
        }
    };
}
