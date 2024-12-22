#pragma once

#include <application.hpp>

#include <ecs/world.hpp>
#include <systems/forward-renderer.hpp>
#include <systems/free-camera-controller.hpp>
#include <systems/rigidbodySystem.hpp>
#include <systems/movement.hpp>
#include <asset-loader.hpp>
#include <systems/InputMovement.hpp>
#include <btBulletCollisionCommon.h>
#include <systems/soundSystem.hpp>
#include <systems/miniaudio.h>

// This state shows how to use the ECS framework and deserialization.
class Playstate : public our::State
{

    our::World world;
    our::ForwardRenderer renderer;
    our::FreeCameraControllerSystem cameraController;
    our::MovementSystem movementSystem;
    our::InputMovementSystem inputMovementSystem;
    our::RigidbodySystem rigidbodySystem;
    our::soundSystem soundSystem;
    
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

        btDiscreteDynamicsWorld *dynamicsWorld = new btDiscreteDynamicsWorld(
            dispatcher,
            broadphase,
            solver,
            collisionConfiguration);

        // Set gravity (Y axis pointing down)
        dynamicsWorld->setGravity(btVector3(0, -9.81f, 0));
        // We initialize the camera controller system since it needs a pointer to the app
        our::Application *appPtr = getApp();
        cameraController.enter(appPtr);
        inputMovementSystem.enter(appPtr);
        rigidbodySystem.enter(dynamicsWorld);
        soundSystem.initialize();
        // Then we initialize the renderer
        auto size = getApp()->getFrameBufferSize();
        renderer.initialize(size, config["renderer"]);
    }

    void onDraw(double deltaTime) override
    {
        // Here, we just run a bunch of systems to control the world logic
        movementSystem.update(&world, (float)deltaTime);
        cameraController.update(&world, (float)deltaTime);
        inputMovementSystem.update(&world, (float)deltaTime);
        rigidbodySystem.update(&world, (float)deltaTime);
        soundSystem.update(&world, (float)deltaTime);
        // And finally we use the renderer system to draw the scene
        renderer.render(&world);

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