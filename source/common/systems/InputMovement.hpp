#pragma once

#include "../ecs/world.hpp"
#include "../components/input.hpp"
#include "../application.hpp"
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>
#include <glm/trigonometric.hpp>
#include <glm/gtx/fast_trigonometry.hpp>
#include<iostream>
using namespace std;
namespace our
{

    class InputMovementSystem
    {
        Application *app;
        glm::vec3 vel;
        glm::vec3 accel;
    public:
        void enter(Application *app)
        {
            this->app = app;
        }

        void update(World *world, float deltaTime)
        {
            // For each entity in the world
            for (auto entity : world->getEntities())
            {
 
                InputComponent *input = entity->getComponent<InputComponent>();
        
                if (input)
                {
                    vel.x = 0;
                    vel.y = 0;
                    vel.z = 0;
                    if (app->getKeyboard().isPressed(GLFW_KEY_UP))
                    {
                        vel.z= 1;
                    }
                    if (app->getKeyboard().isPressed(GLFW_KEY_DOWN))
                    {
                        vel.z = -1;
                    }
                    if (app->getKeyboard().isPressed(GLFW_KEY_LEFT))
                    {
                        vel.x = -1;
                    }
                    if (app->getKeyboard().isPressed(GLFW_KEY_RIGHT))
                    {
                        vel.x = 1;
                    }
                    
                    entity->localTransform.position += deltaTime * vel;
                }
            }
        }
    };

}
