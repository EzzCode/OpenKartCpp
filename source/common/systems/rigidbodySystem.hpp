#pragma once

#include "../ecs/world.hpp"
#include "../components/rigidbody.hpp"
#define TINYOBJLOADER_IMPLEMENTATION
#include <tinyobj/tiny_obj_loader.h>
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>
#include <glm/trigonometric.hpp>
#include <glm/gtx/fast_trigonometry.hpp>
#include <memory>
#include <unordered_map>
#include <stdexcept>

namespace our
{

    class RigidbodySystem
    {
    private:
        btDiscreteDynamicsWorld *dynWorld;
        btRigidBody *createRigidBody(const std::string &meshPath, const btVector3 &position,
                                     const btVector3 &rotation, float mass, const btVector3 &scale = btVector3(1.0f, 1.0f, 1.0f))
        {
            // Load mesh using tinyobj
            tinyobj::attrib_t attrib;
            std::vector<tinyobj::shape_t> shapes;
            std::vector<tinyobj::material_t> materials;
            std::string warn, err;

            bool ret = tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, meshPath.c_str());
            if (!ret)
            {
                throw std::runtime_error("Failed to load mesh: " + meshPath + " " + err);
            }

            // Debug: log the number of vertices and triangles
            std::cout << "Loaded mesh with " << attrib.vertices.size() / 3 << " vertices and "
                      << shapes[0].mesh.indices.size() / 3 << " triangles" << std::endl;
            btTriangleMesh *triangleMesh = new btTriangleMesh();
            for (const auto &shape : shapes)
            {
                for (size_t f = 0; f < shape.mesh.indices.size() / 3; f++)
                {
                    btVector3 vertices[3];
                    for (size_t v = 0; v < 3; v++)
                    {
                        tinyobj::index_t idx = shape.mesh.indices[3 * f + v];
                        vertices[v] = btVector3(
                            attrib.vertices[3 * idx.vertex_index] * scale.getX(),
                            attrib.vertices[3 * idx.vertex_index + 1] * scale.getY(),
                            attrib.vertices[3 * idx.vertex_index + 2] * scale.getZ());
                    }
                    triangleMesh->addTriangle(vertices[0], vertices[1], vertices[2]);
                }
            }

            // Create the collision shape
            btCollisionShape *collisionShape;
            if (triangleMesh->getNumTriangles() > 0)
            {
                if (mass == 0.0f)
                {
                    // Static objects use triangle mesh
                    bool useQuantizedBvhTree = true;
                    collisionShape = new btBvhTriangleMeshShape(triangleMesh, useQuantizedBvhTree);
                }
                else
                {
                    // Dynamic objects use convex hull
                    btConvexHullShape *tmpShape = new btConvexHullShape();
                    for (size_t i = 0; i < attrib.vertices.size(); i += 3)
                    {
                        btVector3 vertex(
                            attrib.vertices[i] * scale.getX(),
                            attrib.vertices[i + 1] * scale.getY(),
                            attrib.vertices[i + 2] * scale.getZ());
                        tmpShape->addPoint(vertex);
                    }
                    collisionShape = tmpShape;
                }
            }
            else
            {
                // Fallback to box shape
                collisionShape = new btBoxShape(btVector3(1.0f, 1.0f, 1.0f));
            }

            // Create the transform
            btTransform transform;
            transform.setIdentity();
            transform.setOrigin(position);
            transform.setRotation(btQuaternion(rotation.x(), rotation.y(), rotation.z()));

            // Setup the rigid body
            btVector3 localInertia(0, 0, 0);
            if (mass != 0.0f)
            {
                collisionShape->calculateLocalInertia(mass, localInertia);
            }

            // Create motion state
            btDefaultMotionState *motionState = new btDefaultMotionState(transform);

            // Create rigid body
            btRigidBody::btRigidBodyConstructionInfo rbInfo(mass, motionState, collisionShape, localInertia);
            btRigidBody *body = new btRigidBody(rbInfo);

            // Add the body to the dynamics world
            dynWorld->addRigidBody(body);

            return body;
        }

    public:
        Application *app;
        void enter(btDiscreteDynamicsWorld *world, Application *app)
        {

            this->app = app;
            if (!world)
                throw std::invalid_argument("Dynamic world cannot be null");
            dynWorld = world;
        }

        void update(World *world, float deltaTime)
        {
            if (!world)
                return;

            for (auto entity : world->getEntities())
            {
                auto *rigidbodyComponent = entity->getComponent<RigidbodyComponent>();
                if (!rigidbodyComponent)
                    continue;

                if (!rigidbodyComponent->addedToWorld)
                {
                    std::string path = "./assets/models/" + rigidbodyComponent->mesh + ".obj";
                    rigidbodyComponent->addedToWorld = true;
                    rigidbodyComponent->rigidbody = createRigidBody(
                        path,
                        btVector3(rigidbodyComponent->position.x, rigidbodyComponent->position.y, rigidbodyComponent->position.z),
                        btVector3(rigidbodyComponent->rotation.x, rigidbodyComponent->rotation.y, rigidbodyComponent->rotation.z),
                        rigidbodyComponent->mass,
                        btVector3(rigidbodyComponent->scale.x, rigidbodyComponent->scale.y, rigidbodyComponent->scale.z));
                }

                if (rigidbodyComponent->input == 1 && rigidbodyComponent->mass != 0.0f)
                {
                    btRigidBody *body = rigidbodyComponent->rigidbody;
                        btVector3 velocity = body->getLinearVelocity();
                        if (velocity.length() > 100.0f)
                        {
                            velocity = velocity.normalized() * 100.0f;
                            body->setLinearVelocity(velocity);
                        }
                    if (app->getKeyboard().isPressed(GLFW_KEY_DOWN))
                    {
                        btVector3 force(0.0f, 0.0f, -500.0f);
                        rigidbodyComponent->rigidbody->applyCentralForce(force);
                    }
                    if (app->getKeyboard().isPressed(GLFW_KEY_UP))
                    {
                        btVector3 force(0.0f, 0.0f, 500.0f);
                        rigidbodyComponent->rigidbody->applyCentralForce(force);
                    }
                    if (app->getKeyboard().isPressed(GLFW_KEY_RIGHT))
                    {
                        btVector3 force(-500.0f, 0.0f, 0.0f);
                        rigidbodyComponent->rigidbody->applyCentralForce(force);
                    }
                    if (app->getKeyboard().isPressed(GLFW_KEY_LEFT))
                    {
                        btVector3 force(500.0f, 0.0f, 0.0f);
                        rigidbodyComponent->rigidbody->applyCentralForce(force);
                    }
                }

                const btTransform &transform = rigidbodyComponent->rigidbody->getWorldTransform();
                const btVector3 &position = transform.getOrigin();
                entity->localTransform.position = glm::vec3(position.x(), position.y(), position.z());
            }
        }
    };

} // namespace our