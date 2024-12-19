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
        glm::vec3 move;

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
                    move.x = 0;
                    move.y = 0;
                    move.z = 0;
                    if (app->getKeyboard().isPressed(GLFW_KEY_UP))
                    {
                        move.y= 1;
                    }
                    if (app->getKeyboard().isPressed(GLFW_KEY_DOWN))
                    {
                        move.y = -1;
                    }
                    if (app->getKeyboard().isPressed(GLFW_KEY_LEFT))
                    {
                        move.x = -1;
                    }
                    if (app->getKeyboard().isPressed(GLFW_KEY_RIGHT))
                    {
                        move.x = 1;
                    }
                    
                    entity->localTransform.position += deltaTime * move;
                }
            }
        }
    };

}
