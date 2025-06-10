#pragma once

#include "../ecs/world.hpp"
#include "../components/rigidbody.hpp"
#include "../application.hpp" // Add this line to include Application type
#define TINYOBJLOADER_IMPLEMENTATION
#include <tinyobj/tiny_obj_loader.h>
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>
#include <glm/trigonometric.hpp>
#include <glm/gtx/fast_trigonometry.hpp>
#include <memory>
#include <iostream>
#include <unordered_map>
#include <stdexcept>
#include <json/json.hpp>
#include <BulletDynamics/Vehicle/btRaycastVehicle.h>
#include "../components/movement.hpp"

namespace our
{

    class RigidbodySystem
    {
    private:
        btDiscreteDynamicsWorld *dynWorld = nullptr;        // Vehicle tuning parameters loaded from config
        struct VehicleTuningConfig
        {
            // Bullet3D btVehicleTuning parameters
            float suspensionStiffness = 5.88f;
            float suspensionCompression = 0.83f;
            float suspensionDamping = 0.88f;
            float maxSuspensionTravelCm = 500.0f;
            float frictionSlip = 10.5f;
            float maxSuspensionForce = 6000.0f;

            // Wheel parameters
            float wheelRestLength = 0.1f;
            float wheelRadius = 0.3f;
            float wheelOffsetX = 0.3f;
            float wheelOffsetZ = 0.4f;
            float wheelFrictionSlip = 10000.0f;
            float wheelRollInfluence = 1.5f;

            // Rigidbody parameters
            float rigidbodyFriction = 0.7f;
            float rigidbodyRollingFriction = 0.1f;
            float linearDamping = 0.1f;
            float angularDamping = 0.5f;

            // Physics parameters
            float maxSpeed = 25.0f;
            float engineForceMin = -700.0f;
            float engineForceMax = 700.0f;
            float brakeForce = 10.0f;
            float gravity = -9.81f;
            float airForce = 15.0f;

            // Steering parameters
            float steeringIncrement = 0.005f;
            float steeringReturnFactor = 0.95f;
            float maxAngleAtLowSpeed = 0.4f;
            float maxAngleAtHighSpeed = 0.07f;
            float speedThreshold = 10.0f;

            // Input parameters
            float engineForceIncrement = 10.0f;
            float brakeForceDefault = 2.0f;

            // Deserialize vehicle tuning configuration from JSON
            void deserialize(const nlohmann::json& data) {
                if (!data.is_object()) return;
                
                // Load Bullet3D parameters
                if (data.contains("bullet3d")) {
                    const auto& b3d = data["bullet3d"];
                    if (b3d.is_object()) {
                        suspensionStiffness = b3d.value("suspensionStiffness", suspensionStiffness);
                        suspensionCompression = b3d.value("suspensionCompression", suspensionCompression);
                        suspensionDamping = b3d.value("suspensionDamping", suspensionDamping);
                        maxSuspensionTravelCm = b3d.value("maxSuspensionTravelCm", maxSuspensionTravelCm);
                        frictionSlip = b3d.value("frictionSlip", frictionSlip);
                        maxSuspensionForce = b3d.value("maxSuspensionForce", maxSuspensionForce);
                    }
                }
                
                // Load wheel parameters
                if (data.contains("wheel")) {
                    const auto& wheel = data["wheel"];
                    if (wheel.is_object()) {
                        wheelRestLength = wheel.value("restLength", wheelRestLength);
                        wheelRadius = wheel.value("radius", wheelRadius);
                        wheelOffsetX = wheel.value("offsetX", wheelOffsetX);
                        wheelOffsetZ = wheel.value("offsetZ", wheelOffsetZ);
                        wheelFrictionSlip = wheel.value("frictionSlip", wheelFrictionSlip);
                        wheelRollInfluence = wheel.value("rollInfluence", wheelRollInfluence);
                    }
                }
                
                // Load rigidbody parameters
                if (data.contains("rigidbody")) {
                    const auto& rb = data["rigidbody"];
                    if (rb.is_object()) {
                        rigidbodyFriction = rb.value("friction", rigidbodyFriction);
                        rigidbodyRollingFriction = rb.value("rollingFriction", rigidbodyRollingFriction);
                        linearDamping = rb.value("linearDamping", linearDamping);
                        angularDamping = rb.value("angularDamping", angularDamping);
                    }
                }
                
                // Load physics parameters
                if (data.contains("physics")) {
                    const auto& physics = data["physics"];
                    if (physics.is_object()) {
                        maxSpeed = physics.value("maxSpeed", maxSpeed);
                        if (physics.contains("engineForceRange")) {
                            const auto& range = physics["engineForceRange"];
                            if (range.is_object()) {
                                engineForceMin = range.value("min", engineForceMin);
                                engineForceMax = range.value("max", engineForceMax);
                            }
                        }
                        brakeForce = physics.value("brakeForce", brakeForce);
                        gravity = physics.value("gravity", gravity);
                        airForce = physics.value("airForce", airForce);
                    }
                }
                
                // Load steering parameters
                if (data.contains("steering")) {
                    const auto& steering = data["steering"];
                    if (steering.is_object()) {
                        steeringIncrement = steering.value("increment", steeringIncrement);
                        steeringReturnFactor = steering.value("returnFactor", steeringReturnFactor);
                        maxAngleAtLowSpeed = steering.value("maxAngleAtLowSpeed", maxAngleAtLowSpeed);
                        maxAngleAtHighSpeed = steering.value("maxAngleAtHighSpeed", maxAngleAtHighSpeed);
                        speedThreshold = steering.value("speedThreshold", speedThreshold);
                    }
                }
                
                // Load input parameters
                if (data.contains("input")) {
                    const auto& input = data["input"];
                    if (input.is_object()) {
                        engineForceIncrement = input.value("engineForceIncrement", engineForceIncrement);
                        brakeForceDefault = input.value("brakeForceDefault", brakeForceDefault);
                    }
                }
            }
        } vehicleTuning;

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
        }    public:
        Application *app;
        
        void enter(btDiscreteDynamicsWorld *world, Application *app, const nlohmann::json& config = nlohmann::json{})
        {
            this->app = app;
            if (!world)
                throw std::invalid_argument("Dynamic world cannot be null");
            dynWorld = world;
            
            // Load vehicle tuning configuration if available
            if (config.contains("vehicleTuning")) {
                vehicleTuning.deserialize(config["vehicleTuning"]);
            }
        }
        //(y: yaw, x: pitch, z: roll)
        void quaternionToEuler(const btQuaternion &quat, float &yaw, float &pitch, float &roll)
        {
            // Constants to avoid magic numbers
            constexpr float SINGULARITY_THRESHOLD = 0.499f; // Close to 0.5
            constexpr float HALF_PI = glm::half_pi<float>();

            // Check for normalized quaternion
            float norm = quat.length2();
            // if (std::abs(norm - 1.0f) > 1e-6f)
            // {
            //     // Handle warning or error for non-normalized quaternion
            //     // Could throw an exception or normalize the quaternion here
            // }

            // Calculate common terms once
            float xy = quat.x() * quat.y();
            float zw = quat.z() * quat.w();
            float test = xy + zw; // sin(pitch) test for gimbal lock

            if (test > SINGULARITY_THRESHOLD)
            {
                // North pole singularity case (pitch = +90 degrees)
                yaw = HALF_PI;
                pitch = 2.0f * std::atan2(quat.x(), quat.w());
                roll = 0.0f;
            }
            else if (test < -SINGULARITY_THRESHOLD)
            {
                // South pole singularity case (pitch = -90 degrees)
                yaw = -HALF_PI;
                pitch = -2.0f * std::atan2(quat.x(), quat.w());
                roll = 0.0f;
            }
            else
            {
                // Regular case
                float xx = quat.x() * quat.x();
                float yy = quat.y() * quat.y();
                float zz = quat.z() * quat.z();

                yaw = std::atan2(2.0f * (quat.x() * quat.w() - quat.y() * quat.z()),
                                 1.0f - 2.0f * (xx + zz));
                pitch = std::asin(2.0f * test);
                roll = std::atan2(2.0f * (quat.y() * quat.w() - quat.x() * quat.z()),
                                  1.0f - 2.0f * (yy + zz));
            }
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
                        rigidbodyComponent->mass,                        btVector3(rigidbodyComponent->scale.x, rigidbodyComponent->scale.y, rigidbodyComponent->scale.z));

                    // Set friction for the rigid body using config values
                    rigidbodyComponent->rigidbody->setFriction(vehicleTuning.rigidbodyFriction);
                    rigidbodyComponent->rigidbody->setRollingFriction(vehicleTuning.rigidbodyRollingFriction);
                }

                // If this object needs a vehicle and has non-zero mass
                if (rigidbodyComponent->input == 1 && rigidbodyComponent->mass != 0.0f)
                {
                    // Create RaycastVehicle once
                    if (!rigidbodyComponent->vehicle)
                    {                        // Vehicle tuning and setup using config values
                        btRaycastVehicle::btVehicleTuning tuning;
                        tuning.m_suspensionStiffness = vehicleTuning.suspensionStiffness;
                        tuning.m_suspensionCompression = vehicleTuning.suspensionCompression;
                        tuning.m_suspensionDamping = vehicleTuning.suspensionDamping;
                        tuning.m_maxSuspensionTravelCm = vehicleTuning.maxSuspensionTravelCm;
                        tuning.m_frictionSlip = vehicleTuning.frictionSlip;
                        tuning.m_maxSuspensionForce = vehicleTuning.maxSuspensionForce;
                        
                        // Configure vehicle rigid body
                        rigidbodyComponent->rigidbody->setActivationState(DISABLE_DEACTIVATION);
                        // Add angular and linear damping using config values
                        rigidbodyComponent->rigidbody->setDamping(vehicleTuning.linearDamping, vehicleTuning.angularDamping);

                        btVehicleRaycaster *raycaster = new btDefaultVehicleRaycaster(dynWorld);
                        rigidbodyComponent->vehicle = new btRaycastVehicle(tuning, rigidbodyComponent->rigidbody, raycaster);
                        rigidbodyComponent->vehicle->setCoordinateSystem(0, 1, 2);

                        btBoxShape *boxShape = static_cast<btBoxShape *>(rigidbodyComponent->rigidbody->getCollisionShape());
                        btVector3 chassisHalfExtents = boxShape->getHalfExtentsWithoutMargin();                        btVector3 wheelDir(0, -1, 0);
                        btVector3 wheelAxle(-1, 0, 0);
                        float restLength = vehicleTuning.wheelRestLength;
                        float radius = vehicleTuning.wheelRadius;
                        float offsetX = vehicleTuning.wheelOffsetX;
                        float offsetZ = vehicleTuning.wheelOffsetZ;
                        //(y,z,x)
                        btVector3 wheelPositions[] = {
                            btVector3(chassisHalfExtents.x() + radius + offsetX, 0, chassisHalfExtents.z() + radius + offsetZ),
                            btVector3(-chassisHalfExtents.x() - radius - offsetX, 0, chassisHalfExtents.z() + radius + offsetZ),
                            btVector3(chassisHalfExtents.x() + radius + offsetX, 0, -chassisHalfExtents.z() - radius - offsetZ),
                            btVector3(-chassisHalfExtents.x() - radius - offsetX, 0, -chassisHalfExtents.z() - radius - offsetZ)};                        // Add wheels with friction using config values
                        for (int i = 0; i < 4; i++)
                        {
                            bool isFrontWheel = (i < 2);
                            rigidbodyComponent->vehicle->addWheel(wheelPositions[i], wheelDir, wheelAxle, restLength, radius, tuning, isFrontWheel);

                            // Set wheel friction using config values
                            btWheelInfo &wheel = rigidbodyComponent->vehicle->getWheelInfo(i);
                            wheel.m_frictionSlip = vehicleTuning.wheelFrictionSlip;
                            wheel.m_rollInfluence = vehicleTuning.wheelRollInfluence;
                        }
                        dynWorld->addVehicle(rigidbodyComponent->vehicle);
                    }

                    btVector3 velocity = rigidbodyComponent->rigidbody->getLinearVelocity();
                    // Limit maximum velocity to prevent excessive speeds using config value
                    if (velocity.length() > vehicleTuning.maxSpeed)
                    {
                        // Normalize velocity and scale to maximum allowed speed
                        velocity = velocity.normalized() * vehicleTuning.maxSpeed;
                        // Apply the clamped velocity back to the rigid body
                        rigidbodyComponent->rigidbody->setLinearVelocity(velocity);
                    }

                    static float engineForce = 0.0f;
                    static float steeringValue = 0.0f;
                    float brakeForce = 0.0f;
                    // Check if the vehicle is in the air by examining the wheel contact points
                    bool isInAir = true;
                    for (int i = 0; i < rigidbodyComponent->vehicle->getNumWheels(); i++)
                    {
                        if (rigidbodyComponent->vehicle->getWheelInfo(i).m_raycastInfo.m_isInContact)
                        {
                            isInAir = false;
                            break;
                        }
                    }                    // Reduce gravity if the vehicle is in the air using config values
                    if (isInAir)
                    {
                        rigidbodyComponent->rigidbody->applyCentralForce(btVector3(0, vehicleTuning.airForce, 0));

                        // Level the car by setting the angular velocity to zero
                        rigidbodyComponent->rigidbody->setAngularVelocity(btVector3(0, 0, 0));

                        dynWorld->setGravity(btVector3(0, vehicleTuning.gravity, 0));
                    }
                    else
                    {
                        dynWorld->setGravity(btVector3(0, vehicleTuning.gravity, 0));
                    }
                    if (app->getKeyboard().isPressed(GLFW_KEY_UP))
                    {
                        engineForce += vehicleTuning.engineForceIncrement;
                    }                    else if (app->getKeyboard().isPressed(GLFW_KEY_DOWN))
                    {
                        engineForce -= vehicleTuning.engineForceIncrement;
                    }
                    else
                    {
                        engineForce = 0.0f;
                        brakeForce = vehicleTuning.brakeForceDefault;
                    }

                    // Modify steering logic using config values
                    if (app->getKeyboard().isPressed(GLFW_KEY_LEFT))
                    {
                        steeringValue += vehicleTuning.steeringIncrement;
                    }
                    else if (app->getKeyboard().isPressed(GLFW_KEY_RIGHT))
                    {
                        steeringValue -= vehicleTuning.steeringIncrement;
                    }                    else
                    {
                        // Add gradual return to center using config value
                        steeringValue *= vehicleTuning.steeringReturnFactor;
                    }

                    // Adjust steering angle limits based on velocity using config values
                    float maxSteeringAngle = glm::mix(vehicleTuning.maxAngleAtLowSpeed, vehicleTuning.maxAngleAtHighSpeed, 
                                                     glm::clamp(velocity.length() / vehicleTuning.speedThreshold, 0.0f, 1.0f));
                    steeringValue = glm::clamp(steeringValue, -maxSteeringAngle, maxSteeringAngle);

                    if (app->getKeyboard().isPressed(GLFW_KEY_SPACE))
                    {
                        brakeForce = vehicleTuning.brakeForce;
                    }
                                        for (int i = 0; i < rigidbodyComponent->vehicle->getNumWheels(); i++)
                    {
                        engineForce = glm::clamp(engineForce, vehicleTuning.engineForceMin, vehicleTuning.engineForceMax);
                        rigidbodyComponent->vehicle->applyEngineForce(engineForce, i);
                        rigidbodyComponent->vehicle->setBrake(brakeForce, i);

                        if (i < 2)
                        {
                            rigidbodyComponent->vehicle->setSteeringValue(steeringValue, i);
                        }
                    }
                    rigidbodyComponent->vehicle->updateVehicle(deltaTime);
                }
                // Get the current transform of the rigid body
                btTransform transform;
                if (rigidbodyComponent->vehicle)
                {

                    rigidbodyComponent->rigidbody->getMotionState()->getWorldTransform(transform);
                    // Get position and rotation
                    btVector3 pos = transform.getOrigin();
                    btQuaternion rot = transform.getRotation();

                    // Convert to Euler angles
                    float yaw, pitch, roll;
                    quaternionToEuler(rot, yaw, roll, pitch);

                    // Update the entity's position and rotation
                    rigidbodyComponent->position = glm::vec3(pos.x(), pos.y(), pos.z());
                    rigidbodyComponent->rotation = glm::vec3(yaw, pitch, roll);

                    entity->localTransform.position = rigidbodyComponent->position;
                    entity->localTransform.position.y -= 0.07f;
                    entity->localTransform.rotation = rigidbodyComponent->rotation;

                    btVector3 velocity = rigidbodyComponent->rigidbody->getLinearVelocity();
                    float steeringValue = rigidbodyComponent->vehicle->getSteeringValue(0);
                    for (auto child : world->getEntities())
                    {
                        if (child->parent == entity)
                        {
                            std::string name = child->name;
                            if (name.find("tire") != std::string::npos)
                            {
                                MovementComponent *movement = child->getComponent<MovementComponent>();
                                if (movement)
                                {
                                    float groundSpeed = velocity.length();
                                    // Get forward direction from velocity
                                    float forwardSpeed = velocity.dot(rigidbodyComponent->rigidbody->getWorldTransform().getBasis().getColumn(2));

                                    if (rigidbodyComponent->vehicle->getWheelInfo(0).m_raycastInfo.m_isInContact)
                                    {
                                        // Apply rotation based on forward/backward movement
                                        float rotationSpeed = groundSpeed * 2 * (forwardSpeed >= 0 ? 1 : -1);
                                        movement->angularVelocity = glm::vec3(rotationSpeed, 0.0f, 0.0f);
                                    }
                                    else
                                    {
                                        movement->angularVelocity = glm::vec3(0.0f, 0.0f, 0.0f);
                                    }
                                }
                                if (name == "tireFront")
                                {
                                    child->localTransform.rotation = glm::vec3(child->localTransform.rotation.x, steeringValue, 0.0f);
                                }
                            }
                        }
                    }
                }
                else if (rigidbodyComponent->rigidbody)
                {
                    rigidbodyComponent->rigidbody->getMotionState()->getWorldTransform(transform);
                    btVector3 pos = transform.getOrigin();
                    btQuaternion rot = transform.getRotation();

                    float yaw, pitch, roll;
                    quaternionToEuler(rot, yaw, pitch, roll);

                    rigidbodyComponent->position = glm::vec3(pos.x(), pos.y(), pos.z());
                    rigidbodyComponent->rotation = glm::vec3(yaw, roll, pitch);

                    entity->localTransform.position = rigidbodyComponent->position;
                    entity->localTransform.rotation = rigidbodyComponent->rotation;
                }
            }
        }
    };

} // namespace our