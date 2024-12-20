#pragma once

#include "../ecs/world.hpp"
#include "../components/input.hpp"
#include "../application.hpp"
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>
#include <glm/trigonometric.hpp>
#include <glm/gtx/fast_trigonometry.hpp>
#include <btBulletDynamicsCommon.h>

namespace our {
    class InputMovementSystem {
    private:
        Application* app;
        
        // Vehicle properties
        float engineForce = 0.0f;
        float brakingForce = 0.0f;
        float steeringAngle = 0.0f;
        
        // Constants
        const float MAX_ENGINE_FORCE = 2000.0f;
        const float MAX_BRAKING_FORCE = 100.0f;
        const float MAX_STEERING_ANGLE = 0.3f;
        const float STEERING_INCREMENT = 0.04f;
        const float STEERING_CLAMP = 0.3f;
        const float VEHICLE_MASS = 800.0f;
        const float DRAG_COEFFICIENT = 0.3f;

    public:
        void enter(Application* app) {
            this->app = app;
        }

        void update(World* world, float deltaTime) {
            for (auto entity : world->getEntities()) {
                InputComponent* input = entity->getComponent<InputComponent>();
                if (!input) continue;

                // Reset forces
                engineForce = 0.0f;
                brakingForce = 0.0f;

                // Forward/Backward movement
                if (app->getKeyboard().isPressed(GLFW_KEY_UP)) {
                    engineForce = MAX_ENGINE_FORCE;
                }
                if (app->getKeyboard().isPressed(GLFW_KEY_DOWN)) {
                    brakingForce = MAX_BRAKING_FORCE;
                }

                // Steering
                if (app->getKeyboard().isPressed(GLFW_KEY_LEFT)) {
                    steeringAngle += STEERING_INCREMENT;
                    if (steeringAngle > STEERING_CLAMP) 
                        steeringAngle = STEERING_CLAMP;
                } else if (app->getKeyboard().isPressed(GLFW_KEY_RIGHT)) {
                    steeringAngle -= STEERING_INCREMENT;
                    if (steeringAngle < -STEERING_CLAMP) 
                        steeringAngle = -STEERING_CLAMP;
                } else {
                    // Return steering to center
                    steeringAngle *= 0.9f;
                }

                // Calculate vehicle physics
                glm::vec3 forwardDir = glm::normalize(
                    glm::vec3(sin(entity->localTransform.rotation.y), 
                             0, 
                             cos(entity->localTransform.rotation.y))
                );
                
                // Apply forces
                glm::vec3 acceleration = forwardDir * (engineForce / VEHICLE_MASS);
                glm::vec3 velocity = forwardDir * engineForce * deltaTime;
                
                // Apply drag
                velocity *= (1.0f - DRAG_COEFFICIENT * deltaTime);
                
                // Apply braking
                if (brakingForce > 0) {
                    velocity *= (1.0f - brakingForce * deltaTime);
                }

                // Update position and rotation
                entity->localTransform.position += velocity * deltaTime;
                entity->localTransform.rotation.y += steeringAngle * velocity.length() * deltaTime;
            }
        }
    };
}