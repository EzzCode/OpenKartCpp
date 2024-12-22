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
#include <BulletDynamics/Vehicle/btRaycastVehicle.h>

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

            // Create the rigid body if not already in the world
            if (!rigidbodyComponent->addedToWorld)
            {
                rigidbodyComponent->addedToWorld = true;
                std::string path = "./assets/models/" + rigidbodyComponent->mesh + ".obj";
                rigidbodyComponent->rigidbody = createRigidBody(
                path,
                btVector3(rigidbodyComponent->position.x, rigidbodyComponent->position.y, rigidbodyComponent->position.z),
                btVector3(rigidbodyComponent->rotation.x, rigidbodyComponent->rotation.y, rigidbodyComponent->rotation.z),
                rigidbodyComponent->mass,
                btVector3(rigidbodyComponent->scale.x, rigidbodyComponent->scale.y, rigidbodyComponent->scale.z));
                
                // Set friction for the rigid body
                rigidbodyComponent->rigidbody->setFriction(0.7f);
                rigidbodyComponent->rigidbody->setRollingFriction(0.1f);
            }

            // If this object needs a vehicle and has non-zero mass
            if (rigidbodyComponent->input == 1 && rigidbodyComponent->mass != 0.0f)
            {
                // Create RaycastVehicle once
                if (!rigidbodyComponent->vehicle)
                {
                // Vehicle tuning and setup
                btRaycastVehicle::btVehicleTuning tuning;
                tuning.m_suspensionStiffness = 50.0f;
                tuning.m_suspensionCompression = 2.3f;
                tuning.m_suspensionDamping = 2.4f;
                tuning.m_maxSuspensionTravelCm = 200.0f;
                tuning.m_frictionSlip = 1000.0f;
                tuning.m_maxSuspensionForce = 10000.0f;

                // Configure vehicle rigid body
                rigidbodyComponent->rigidbody->setActivationState(DISABLE_DEACTIVATION);
                rigidbodyComponent->rigidbody->setAngularFactor(btVector3(0, 1, 0));

                btVehicleRaycaster *raycaster = new btDefaultVehicleRaycaster(dynWorld);
                rigidbodyComponent->vehicle = new btRaycastVehicle(tuning, rigidbodyComponent->rigidbody, raycaster);
                rigidbodyComponent->vehicle->setCoordinateSystem(0, 1, 2);

                dynWorld->addVehicle(rigidbodyComponent->vehicle);

                btBoxShape* boxShape = static_cast<btBoxShape*>(rigidbodyComponent->rigidbody->getCollisionShape());
                btVector3 chassisHalfExtents = boxShape->getHalfExtentsWithoutMargin();
                
                btVector3 wheelDir(0, -1, 0);
                btVector3 wheelAxle(-1, 0, 0);
                float restLength = 0.6f;
                float radius = 0.4f;
                btVector3 wheelPositions[] = {
                    btVector3(chassisHalfExtents.x() + radius, -chassisHalfExtents.y() - radius, chassisHalfExtents.z() + radius),
                    btVector3(-chassisHalfExtents.x() - radius, -chassisHalfExtents.y() - radius, chassisHalfExtents.z() + radius),
                    btVector3(chassisHalfExtents.x() + radius, -chassisHalfExtents.y() - radius, -chassisHalfExtents.z() - radius),
                    btVector3(-chassisHalfExtents.x() - radius, -chassisHalfExtents.y() - radius, -chassisHalfExtents.z() - radius)};

                // Add wheels with friction
                for (int i = 0; i < 4; i++)
                {
                    bool isFrontWheel = (i < 2);
                    rigidbodyComponent->vehicle->addWheel(wheelPositions[i], wheelDir, wheelAxle, restLength, radius, tuning, isFrontWheel);
                    
                    // Set wheel friction
                    btWheelInfo& wheel = rigidbodyComponent->vehicle->getWheelInfo(i);
                    wheel.m_frictionSlip = 1000.0f;
                    wheel.m_rollInfluence = 0.7f;
                }
                }

                btVector3 velocity = rigidbodyComponent->rigidbody->getLinearVelocity();
                if (velocity.length() > 10.0f)
                {
                velocity = velocity.normalized() * 10.0f;
                rigidbodyComponent->rigidbody->setLinearVelocity(velocity);
                }

                static float engineForce = 0.0f;
                static float steeringValue = 0.0f;
                float brakeForce = 0.0f;

                if (app->getKeyboard().isPressed(GLFW_KEY_UP))
                {
                engineForce += 1.0f;
                }
                else if (app->getKeyboard().isPressed(GLFW_KEY_DOWN))
                {
                engineForce -= 1.0f;
                }
                else
                {
                engineForce = 0.0f;
                }

                if (app->getKeyboard().isPressed(GLFW_KEY_LEFT))
                {
                steeringValue += 0.01f;
                }
                else if (app->getKeyboard().isPressed(GLFW_KEY_RIGHT))
                {
                steeringValue -= 0.01f;
                }
                else
                {
                steeringValue = 0.0f;
                }

                if (app->getKeyboard().isPressed(GLFW_KEY_SPACE))
                {
                brakeForce = 100.0f;
                }
                else
                {
                brakeForce = 0.0f;
                }

                for (int i = 0; i < rigidbodyComponent->vehicle->getNumWheels(); i++)
                {
                rigidbodyComponent->vehicle->applyEngineForce(engineForce, i);
                rigidbodyComponent->vehicle->setBrake(brakeForce, i);
                std::cout << "Engine force: " << engineForce << std::endl;
                std::cout << "Brake force: " << brakeForce << std::endl;
        
                if (i < 2)
                {
                    steeringValue = glm::clamp(steeringValue, -0.3f, 0.3f);
                    rigidbodyComponent->vehicle->setSteeringValue(steeringValue, i);
                    std::cout << "Steering value: " << steeringValue << std::endl;
                }
                }
            }

            btTransform transform;
            if(rigidbodyComponent->vehicle)
                transform = rigidbodyComponent->vehicle->getChassisWorldTransform();
            else
                transform = rigidbodyComponent->rigidbody->getWorldTransform();

            const btVector3 &pos = transform.getOrigin();
            rigidbodyComponent->position = glm::vec3(pos.x(), pos.y(), pos.z());
            entity->localTransform.position = glm::vec3(pos.x(), pos.y(), pos.z());
            btQuaternion quat = transform.getRotation();
            btScalar yaw, pitch, roll;
            quat.getEulerZYX(yaw, pitch, roll);
            rigidbodyComponent->rotation = glm::vec3(roll, pitch, yaw);
            entity->localTransform.rotation = glm::vec3(roll, pitch, yaw);
            }
        }
    };

} // namespace our