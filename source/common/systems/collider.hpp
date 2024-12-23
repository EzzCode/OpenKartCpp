#pragma once

#include "../ecs/world.hpp"
#include "../components/collider.hpp"
#include "../components/movement.hpp"
#include "../application.hpp"
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>
#include <glm/trigonometric.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtx/euler_angles.hpp>
#include <glm/gtx/fast_trigonometry.hpp>
#include <vector>
#include <iostream>

namespace our {
    class ColliderSystem {
        Application *app; // The application in which the state runs
        glm::vec3 gravity = glm::vec3(0.0f, -9.81f, 0.0f); // Define gravity
        float maxSpeed = 25.0f; // Define the maximum speed
        float minSpeed = -25.0f; // Define the minimum speed
        float acceleration = 5.0f; // Define the acceleration rate
        int turnspertramp = 0; // Counter for trampoline turns
        bool gravityOn = true; // Gravity toggle
        int hitcounter = 0; // Counter for hits

    public:
        // when a state enters, it should call this function and give it the pointer to the application
        void enter(Application *app) {
            this->app = app;
        }

        bool checkCollision(Entity *entity1, Entity *entity2) {
            ColliderComponent *collider1 = entity1->getComponent<ColliderComponent>();
            ColliderComponent *collider2 = entity2->getComponent<ColliderComponent>();

            if (!collider1 || !collider2) {
                return false;
            }

            // Get the transformation matrices of the entities
            glm::mat4 transform1 = entity1->localTransform.toMat4();
            glm::mat4 transform2 = entity2->localTransform.toMat4();

            // Get the scaled dimensions of the colliders
            glm::vec3 scale1 = entity1->localTransform.scale;
            glm::vec3 scale2 = entity2->localTransform.scale;

            float x_diff1 = collider1->x_diff * scale1.x;
            float y_diff1 = collider1->y_diff * scale1.y;
            float z_diff1 = collider1->z_diff * scale1.z;

            float x_diff2 = collider2->x_diff * scale2.x;
            float y_diff2 = collider2->y_diff * scale2.y;
            float z_diff2 = collider2->z_diff * scale2.z;

            // Get the positions of the entities
            glm::vec3 pos1 = glm::vec3(transform1[3]);
            glm::vec3 pos2 = glm::vec3(transform2[3]);

            // Calculate the differences in positions
            float x_diff = abs(pos1.x - pos2.x);
            float y_diff = abs(pos1.y - pos2.y);
            float z_diff = abs(pos1.z - pos2.z);

            // Check for collision
            return x_diff <= (x_diff1 + x_diff2) && y_diff <= (y_diff1 + y_diff2) && z_diff <= (z_diff1 + z_diff2);
        }

        // this should be called every frame to update all entities containing a colliderComponent
        void update(World *world, float deltaTime) {
            // create a vector of all collider components
            std::vector<ColliderComponent *> colliders;
            for (auto entity : world->getEntities()) {
                ColliderComponent *collider = entity->getComponent<ColliderComponent>();
                if (collider) {
                    colliders.push_back(collider);
                }
            }

            // iterate through all colliders
            for (int i = 0; i < colliders.size(); i++) {
                // get the first collider
                ColliderComponent *collider1 = colliders[i];

                // Reset the collision flag for each entity
                bool isCollidingWithAction1 = false;

                // iterate through all colliders again
                for (int j = 0; j < colliders.size(); j++) {
                    // if the colliders are the same, skip
                    if (i == j) {
                        continue;
                    }

                    // get the second collider
                    ColliderComponent *collider2 = colliders[j];

                    // get the entity that we found via getOwner of collider
                    Entity *entity1 = collider1->getOwner();
                    Entity *entity2 = collider2->getOwner();
                    MovementComponent* movement = entity1->getComponent<MovementComponent>();
                    //obstacles collision 
                    if (checkCollision(entity1, entity2))
                    {
                        if (collider1->action==0 && collider2->action==4)
                        {
                            if (entity2->name == "wall")
                            {     
                                hitcounter++;
                                if (hitcounter>1)
                                {
                                    hitcounter = 0;
                            world->markForRemoval(entity2);
                                }
                                else
                                {
                            entity1->localTransform.position.z = -20.0f;
                                }
                                                      
                            }
                            else if (entity2->name == "grass")
                            {
                                std::cout << "grass" << std::endl;
                                movement->linearVelocity *= 0.1f; // Reduce the speed by half
                            }
                            
                        }
                        
                    }
                    
                    //action 0 for kart 1 for glass 2 for trampoline 3 for moon (endline) 4 for wall obstacle
                    if (checkCollision(entity1, entity2)) {
                        if (collider1->action == 0 && collider2->action == 1) 
                        { // 0 for kart 1 for glass 
                            isCollidingWithAction1 = true;
 
                           if (entity2->name== "wall1")
                           {

                            if(gravityOn)
                            {
                            movement->linearVelocity.y = 0.0f;
                            }
                            // Ensure the kart remains grounded

                            // Gradually increase the speed until it reaches the maximum speed
                            // if (movement->linearVelocity.z < maxSpeed) {
                            //     movement->linearVelocity.z += acceleration * deltaTime;
                            //     if (movement->linearVelocity.z > maxSpeed) {
                            //         movement->linearVelocity.z = maxSpeed;
                            //     }
                            // }
                           }
                           else if (entity2->name== "wall2")
                           {
                            std::cout << "wall2 " << std::endl;
                            //entity1->localTransform.rotation.y = glm::radians(90.0f);
                            //entity1->localTransform.position.y = -5;
                           // movement->linearVelocity.z = 0.0f;
                            movement->linearVelocity.y = 0.0f;

                            // Ensure the kart remains grounded
                            if(gravityOn)
                            {
                            movement->linearVelocity.y = 0.0f;
                            }
                            // Gradually dec the speed until it reaches the minimum speed
                            // if (movement->linearVelocity.x < maxSpeed) {
                            //     movement->linearVelocity.x += acceleration * deltaTime;
                            //     if (movement->linearVelocity.x > maxSpeed) {
                            //         movement->linearVelocity.x = maxSpeed;
                            //     }
                            // }
                           }
                           else if (entity2->name== "wall3")
                           {
                            std::cout << "wall3 " << std::endl;

                            //entity1->localTransform.rotation.y = glm::radians(90.0f);
                           // movement->linearVelocity.x = 0.0f;
                            // Ensure the kart remains grounded
                            if(gravityOn)
                            {
                            movement->linearVelocity.y = 0.0f;
                            }                             
                           // movement->linearVelocity.z = -10.0f;                          
                           
                           }
                           else if (entity2->name== "wall5")
                           {
                          // movement->linearVelocity.x = 0.0f;
                            // Ensure the kart remains grounded
                            movement->linearVelocity.y = 10.0f;  
                           // movement->linearVelocity.z = -10.0f;                          
                           }
                           else if (entity2->name== "wall6") // to jump and transform
                           {
                            std::cout << "wall6 " << std::endl;
                           // entity1->localTransform.rotation.y = glm::radians(180.0f);
                          // movement->linearVelocity.x = 0.0f;
                            // Ensure the kart remains grounded
                            movement->linearVelocity.y = 10.0f;  
                          //  movement->linearVelocity.z = -10.0f;                          
                           }
                           else if (entity2->name== "wall4")
                           {
                            if(gravityOn)
                            {
                            movement->linearVelocity.y = 0.0f;
                            }                            // Ensure the kart remains grounded
                            // Gradually dec the speed until it reaches the minimum speed
                            // if (movement->linearVelocity.z > minSpeed) {
                            //     movement->linearVelocity.z -= acceleration * deltaTime;
                            //     if (movement->linearVelocity.z < minSpeed) {
                            //         movement->linearVelocity.z = minSpeed;
                            //     }
                            // }                             
                           }
                        }
 
                    }
                    if (checkCollision(entity1, entity2))
                    {
                        if(collider1->action == 0 && collider2->action == 3)
                        {
                            if (gravityOn)
                            {
                            std::cout << "WINNER " << std::endl;
                            }
                            

                            
                            movement->linearVelocity.x = 0.0f;
                            movement->linearVelocity.y = 0.0f;  
                            movement->linearVelocity.z = 0.0f;
                            gravityOn = 0; 
                        }
                    }
                    
                     if (collider1->action == 0 && collider2->action == 2) 
                        { // 0 for kart 2 for trampoline
                        if (checkCollision(entity1, entity2))
                        {
                                gravityOn = 0;
                               // movement->linearVelocity.y = 10.0f;
                      
                                 }
                                                   
                         }
                    
                }
                                turnspertramp++;                
                                if (turnspertramp>5)
                                {
                                    turnspertramp = 0;
                                gravityOn = 1;
                                } 
                // Apply gravity if not colliding with action 1
                if (!isCollidingWithAction1) {
                    MovementComponent* movement = colliders[i]->getOwner()->getComponent<MovementComponent>();
                    if (movement) {
                        movement->linearVelocity += gravity * deltaTime;
                    }
                }
            }

        }
    };
}