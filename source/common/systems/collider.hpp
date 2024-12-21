#include "../ecs/world.hpp"
#include "../components/collider.hpp"

#include "../application.hpp"

#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>
#include <glm/trigonometric.hpp>
#include <glm/gtx/fast_trigonometry.hpp>
#include <vector>
#include <iostream>

namespace our
{
    class ColliderSystem
    {
        Application *app; // The application in which the state runs

    public:
        // when a state enters, it should call this function and give it the pointer to the application
        void enter(Application *app)
        {
            this->app = app;
        }

bool checkCollision(Entity *entity1, Entity *entity2)
{
    ColliderComponent *collider1 = entity1->getComponent<ColliderComponent>();
    ColliderComponent *collider2 = entity2->getComponent<ColliderComponent>();

    // get the diffrence in x position  between the two entities
    float x_diff = entity1->localTransform.position.x - entity2->localTransform.position.x;
    // get the diffrence in y position  between the two entities
    float y_diff = entity1->localTransform.position.y - entity2->localTransform.position.y;
    // get the diffrence in z position  between the two entities
    float z_diff = entity1->localTransform.position.z - entity2->localTransform.position.z;

    // check if the difference is negative then make it positive
    if (x_diff < 0)
    {
        x_diff = -1 * x_diff;
    }
    if (y_diff < 0)
    {
        y_diff = -1 * y_diff;
    }
    if (z_diff < 0)
    {
        z_diff = -1 * z_diff;
    }
    // if the difference in x, y, and z is less than the sum of the two colliders' x_diff, y_diff, and z_diff, then they are colliding
    return x_diff <= (collider1->x_diff + collider2->x_diff) && y_diff <= (collider1->y_diff + collider2->y_diff) && z_diff <= (collider1->z_diff + collider2->z_diff);
}
        // this should be called every frame to update all entities containing a colliderComponent
        void update(World *world, float deltaTime)
        {
            // create a vector of all collider components
            std::vector<ColliderComponent *> colliders;
            for (auto entity : world->getEntities())
            {
                ColliderComponent *collider = entity->getComponent<ColliderComponent>();
                if (collider)
                {
                    colliders.push_back(collider);
                }
            }

            // iterate through all colliders
            for (int i = 0; i < colliders.size(); i++)
            {
                // get the first collider
                ColliderComponent *collider1 = colliders[i];

                // iterate through all colliders again
                for (int j = 0; j < colliders.size(); j++)
                {
                    // if the colliders are the same, skip
                    if (i == j)
                    {
                        continue;
                    }

                    // get the second collider
                    ColliderComponent *collider2 = colliders[j];

                    // get the entity that we found via getOwner of collider
                    Entity *entity1 = collider1->getOwner();
                    Entity *entity2 = collider2->getOwner();

                    if (entity1->name== "kart" && entity2->name == "track")
                    {
                            std::cout << "Entity kart (x)" <<entity1->localTransform.position.x<< "Entity 1 (y)" <<entity1->localTransform.position.y<< "Entity 1 (z)" <<entity1->localTransform.position.z<< std::endl;
                            std::cout << "Entity track 2 (x)" <<entity2->localTransform.position.x<< "Entity 2 (y)" <<entity2->localTransform.position.y<< "Entity 2 (z)" <<entity2->localTransform.position.z<< std::endl;
                    }
                    

                    if (checkCollision(entity1, entity2))
                    {

                        if (collider1->action == 0 && collider2->action == 1) // 0 for kart 1 for wall
                        {
                            std::cout << "Obstacle collision 1" << std::endl;

                            world->markForRemoval(entity2);
                        }

                    }
                }
            }
        }
    };
}