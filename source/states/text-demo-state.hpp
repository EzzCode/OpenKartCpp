#pragma once

#include <asset-loader.hpp>
#include <ecs/world.hpp>
#include <systems/forward-renderer.hpp>
#include <systems/text-render-system.hpp>
#include <components/text-renderer.hpp>
#include <application.hpp>
#include <GLFW/glfw3.h>
#include <sstream>
#include <iomanip>

// This state demonstrates text rendering with FreeType using ECS
class TextRenderingDemoState : public our::State
{

    our::World world;
    our::ForwardRenderer renderer;
    our::TextRenderSystem textRenderSystem;

    // Text entities for various demos
    our::Entity *titleEntity = nullptr;
    our::Entity *normalTextEntity = nullptr;
    our::Entity *largeTextEntity = nullptr;
    our::Entity *smallTextEntity = nullptr;
    our::Entity *centeredTextEntity = nullptr;
    our::Entity *redTextEntity = nullptr;
    our::Entity *greenTextEntity = nullptr;
    our::Entity *blueTextEntity = nullptr;
    our::Entity *instructionEntity = nullptr;
    our::Entity *fpsEntity = nullptr;
    our::Entity *animatedTextEntity = nullptr;

    float animationTime = 0.0f;

    void onInitialize() override
    {
        // First of all, we get the scene configuration from the app config
        auto &config = getApp()->getConfig()["scene"];
        // If we have assets in the scene config, we deserialize them
        if (config.contains("assets"))
        {
            our::deserializeAllAssets(config["assets"]);
        }

        glm::ivec2 size = getApp()->getFrameBufferSize();
        renderer.initialize(size, config["renderer"]);
        textRenderSystem.initialize(size);

        // Create text entities for demonstration
        createTextEntities();
    }
    void onDraw(double deltaTime) override
    {
        // Update animation time
        animationTime += deltaTime;

        // Clear the screen
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glClearColor(0.1f, 0.1f, 0.2f, 1.0f);

        // Update animated text
        updateAnimatedText();
        updateFPSText(deltaTime);

        // Render all text entities using the text render system
        textRenderSystem.render(&world);

        // Handle input
        auto &keyboard = getApp()->getKeyboard();
        if (keyboard.justPressed(GLFW_KEY_ESCAPE))
        {
            getApp()->changeState("menu");
        }
    }

private:
    void createTextEntities()
    {
        auto size = getApp()->getFrameBufferSize();

        // Title text
        titleEntity = createTextEntity("FreeType Text Rendering Demo", 50.0f, size.y - 100.0f, 1.0f, glm::vec3(1.0f, 1.0f, 1.0f));

        // Normal text samples
        normalTextEntity = createTextEntity("This is normal text", 50.0f, size.y - 200.0f, 1.0f, glm::vec3(0.8f, 0.8f, 0.8f));
        largeTextEntity = createTextEntity("Large Text!", 50.0f, size.y - 320.0f, 1.5f, glm::vec3(1.0f, 0.5f, 0.0f));
        smallTextEntity = createTextEntity("Small text", 50.0f, size.y - 420.0f, 0.5f, glm::vec3(0.5f, 1.0f, 0.5f));

        // Centered text
        centeredTextEntity = createTextEntity("Centered Text", size.x / 2.0f, size.y / 2.0f, 1.2f, glm::vec3(1.0f, 0.0f, 1.0f), true);

        // Color samples
        redTextEntity = createTextEntity("Red Text", 50.0f, 150.0f, 1.0f, glm::vec3(1.0f, 0.0f, 0.0f));
        greenTextEntity = createTextEntity("Green Text", 50.0f, 100.0f, 1.0f, glm::vec3(0.0f, 1.0f, 0.0f));
        blueTextEntity = createTextEntity("Blue Text", 50.0f, 50.0f, 1.0f, glm::vec3(0.0f, 0.0f, 1.0f));

        // Instructions
        instructionEntity = createTextEntity("Press ESC to return to menu", size.x - 400.0f, 25.0f, 0.7f, glm::vec3(0.7f, 0.7f, 0.7f));

        // FPS counter
        fpsEntity = createTextEntity("FPS: 0.0", size.x - 150.0f, size.y - 50.0f, 0.8f, glm::vec3(1.0f, 1.0f, 0.0f));

        // Animated text
        animatedTextEntity = createTextEntity("ANIMATED TEXT DEMO", size.x / 2.0f, size.y - 300.0f, 1.5f, glm::vec3(1.0f, 0.5f, 0.0f), true);
    }

    our::Entity *createTextEntity(const std::string &text, float x, float y, float scale = 1.0f,
                                  const glm::vec3 &color = glm::vec3(1.0f), bool centered = false)
    {
        our::Entity *entity = world.add();
        entity->localTransform.position = glm::vec3(x, y, 0.0f);
        entity->localTransform.scale = glm::vec3(scale, scale, 1.0f);

        auto textRenderer = entity->addComponent<our::TextRendererComponent>();
        textRenderer->text = text;
        textRenderer->color = color;
        textRenderer->scale = scale;
        textRenderer->centered = centered;
        textRenderer->visible = true;

        return entity;
    }

    void updateAnimatedText()
    {
        if (animatedTextEntity)
        {
            auto textRenderer = animatedTextEntity->getComponent<our::TextRendererComponent>();
            if (textRenderer)
            {
                // Pulsing scale animation
                float pulseScale = 1.5f + 0.3f * sin(animationTime * 3.0f);
                animatedTextEntity->localTransform.scale = glm::vec3(pulseScale, pulseScale, 1.0f);

                // Color animation (rainbow effect)
                textRenderer->color = glm::vec3(
                    0.5f + 0.5f * sin(animationTime * 2.0f),
                    0.5f + 0.5f * sin(animationTime * 2.0f + 2.094f),
                    0.5f + 0.5f * sin(animationTime * 2.0f + 4.188f));
            }
        }
    }

    void updateFPSText(double deltaTime)
    {
        static int frameCount = 0;
        static double lastTime = 0.0;
        static double currentFPS = 0.0;

        frameCount++;
        lastTime += deltaTime;

        if (lastTime >= 1.0)
        {
            currentFPS = frameCount / lastTime;
            frameCount = 0;
            lastTime = 0.0;

            if (fpsEntity)
            {
                auto textRenderer = fpsEntity->getComponent<our::TextRendererComponent>();
                if (textRenderer)
                {
                    std::ostringstream fpsStream;
                    fpsStream << "FPS: " << std::fixed << std::setprecision(1) << currentFPS;
                    textRenderer->text = fpsStream.str();
                }
            }
        }
    }

public:
    void onDestroy() override
    {
        textRenderSystem.destroy();
        renderer.destroy();
        world.clear();
        our::clearAllAssets();
    }

    void onImmediateGui() override
    {
        // You can add ImGui controls here if needed
    }
};
