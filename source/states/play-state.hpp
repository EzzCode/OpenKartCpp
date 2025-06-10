#pragma once

#include <application.hpp>

#include <ecs/world.hpp>
#include <systems/forward-renderer.hpp>
#include <systems/free-camera-controller.hpp>
#include <systems/rigidbodySystem.hpp>
#include<systems/ColliderSystem.hpp>
#include <systems/movement.hpp>
#include <asset-loader.hpp>
#include <systems/InputMovement.hpp>
#include <btBulletCollisionCommon.h>
#include <systems/soundSystem.hpp>
#include <systems/miniaudio.h>
#include <systems/race-system.hpp>
#include <systems/hud-system.hpp>

#include <btBulletDynamicsCommon.h>
// This state shows how to use the ECS framework and deserialization.
class Playstate : public our::State
{

    our::World world;
    our::ForwardRenderer renderer;
    our::FreeCameraControllerSystem cameraController;
    our::MovementSystem movementSystem;
    our::InputMovementSystem inputMovementSystem;    our::RigidbodySystem rigidbodySystem;
    our::soundSystem soundSystem;
    our::ColliderSystem colliderSystem;
    our::RaceSystem raceSystem;
    our::HUDSystem hudSystem;
    btDiscreteDynamicsWorld *dynamicsWorld;
    void onInitialize() override
    {
        
        // First of all, we get the scene configuration from the app config
        auto &config = getApp()->getConfig()["scene"];
        // If we have assets in the scene config, we deserialize them
        if (config.contains("assets"))
        {
            our::deserializeAllAssets(config["assets"]);
        }
        // If we have a world in the scene config, we use it to populate our world
        if (config.contains("world"))
        {
            world.deserialize(config["world"]);
        }
        btBroadphaseInterface *broadphase = new btDbvtBroadphase();
        btDefaultCollisionConfiguration *collisionConfiguration = new btDefaultCollisionConfiguration();
        btCollisionDispatcher *dispatcher = new btCollisionDispatcher(collisionConfiguration);
        btSequentialImpulseConstraintSolver *solver = new btSequentialImpulseConstraintSolver();

        dynamicsWorld = new btDiscreteDynamicsWorld(
            dispatcher,
            broadphase,
            solver,
            collisionConfiguration);

        // Set gravity (Y axis pointing down)
        dynamicsWorld->setGravity(btVector3(0, -9.81f, 0));

        // Create ground plane
        btCollisionShape *groundShape = new btBoxShape(btVector3(500, 0.5f, 500)); // Half-extents
        btDefaultMotionState *groundMotionState = new btDefaultMotionState(
            btTransform(btQuaternion(0, 0, 0, 1), btVector3(0, 1.0f, 0)));

        btRigidBody::btRigidBodyConstructionInfo groundRigidBodyCI(
            0, // mass = 0 makes it static
            groundMotionState,
            groundShape,
            btVector3(0, 0, 0) // local inertia
        );

        btRigidBody *groundRigidBody = new btRigidBody(groundRigidBodyCI);
        dynamicsWorld->addRigidBody(groundRigidBody);
        our::Application *appPtr = getApp();
        cameraController.enter(appPtr);
        // InputMovementSystem disabled to avoid conflicts with RigidbodySystem
        // inputMovementSystem.enter(appPtr);        
        
        // Pass vehicleTuning config section directly without copying - more efficient O(1) access
        const auto& fullConfig = getApp()->getConfig();
        auto vehicleTuningIt = fullConfig.find("vehicleTuning");
        if (vehicleTuningIt != fullConfig.end()) {
            rigidbodySystem.enter(dynamicsWorld, appPtr, *vehicleTuningIt);
        } else {
            rigidbodySystem.enter(dynamicsWorld, appPtr);
        }
        
        soundSystem.initialize();
          // Initialize race and HUD systems
        raceSystem.enter(appPtr);
        
        // Then we initialize the renderer
        auto size = getApp()->getFrameBufferSize();
        renderer.setWorld(dynamicsWorld);
        renderer.initialize(size, config["renderer"]);
        
        // Initialize HUD system with renderer reference
        hudSystem.enter(appPtr, &raceSystem, &renderer);
    }

    void onDraw(double deltaTime) override
    {        // Here, we just run a bunch of systems to control the world logic
        // Update physics with optimized timestep for better performance
    
        // Use adaptive timestep with more substeps for better performance and stability
        dynamicsWorld->stepSimulation((float)deltaTime, 10, 1.0f / 120.0f);
        
        movementSystem.update(&world, (float)deltaTime);
        cameraController.update(&world, (float)deltaTime);
        
        // Only use RigidbodySystem for physics-based movement
        // InputMovementSystem is disabled to avoid conflicts
        rigidbodySystem.update(&world, (float)deltaTime);
        soundSystem.update(&world, (float)deltaTime);
        colliderSystem.update(&world, (float)deltaTime);
        
        // Update race system
        raceSystem.update(&world, (float)deltaTime);

        // And finally we use the renderer system to draw the scene
        renderer.render(&world);
        
        // Update and render HUD
        hudSystem.update(&world, (float)deltaTime);
        hudSystem.render();
        // Get a reference to the keyboard object
        auto &keyboard = getApp()->getKeyboard();

        if (keyboard.justPressed(GLFW_KEY_ESCAPE))
        {
            // If the escape  key is pressed in this frame, go to the play state
            getApp()->changeState("menu");
        }
    }

    void onDestroy() override
    {
        // We destroy the sound system
        soundSystem.destroy();
        // Don't forget to destroy the renderer
        renderer.destroy();
        // On exit, we call exit for the camera controller system to make sure that the mouse is unlocked
        cameraController.exit();
        // Clear the world
        world.clear();
        // and we delete all the loaded assets to free memory on the RAM and the VRAM
        our::clearAllAssets();
    }
};