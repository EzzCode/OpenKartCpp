#pragma once

#include <asset-loader.hpp>
#include <ecs/world.hpp>
#include <systems/forward-renderer.hpp>
#include <application.hpp>
#include <GLFW/glfw3.h>

// This state demonstrates text rendering with FreeType
class TextRenderingDemoState: public our::State {

    our::ForwardRenderer renderer;
    
    void onInitialize() override {
        // First of all, we get the scene configuration from the app config
        auto& config = getApp()->getConfig()["scene"];
        // If we have assets in the scene config, we deserialize them
        if(config.contains("assets")){
            our::deserializeAllAssets(config["assets"]);
        }

        glm::ivec2 size = getApp()->getFrameBufferSize();
        renderer.initialize(size, config["renderer"]);
    }

    void onDraw(double deltaTime) override {
        // Clear the screen
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glClearColor(0.1f, 0.1f, 0.2f, 1.0f);
        
        auto size = getApp()->getFrameBufferSize();
        
        // Render different text examples
        renderer.renderText("FreeType Text Rendering Demo", 50.0f, size.y - 100.0f, 1.0f, glm::vec3(1.0f, 1.0f, 1.0f));
        
        renderer.renderText("This is normal text", 50.0f, size.y - 200.0f, 1.0f, glm::vec3(0.8f, 0.8f, 0.8f));
        
        renderer.renderText("Large Text!", 50.0f, size.y - 320.0f, 1.5f, glm::vec3(1.0f, 0.5f, 0.0f));
        
        renderer.renderText("Small text", 50.0f, size.y - 420.0f, 0.5f, glm::vec3(0.5f, 1.0f, 0.5f));
        
        renderer.renderTextCentered("Centered Text", size.x / 2.0f, size.y / 2.0f, 1.2f, glm::vec3(1.0f, 0.0f, 1.0f));
        
        renderer.renderText("Red Text", 50.0f, 150.0f, 1.0f, glm::vec3(1.0f, 0.0f, 0.0f));
        renderer.renderText("Green Text", 50.0f, 100.0f, 1.0f, glm::vec3(0.0f, 1.0f, 0.0f));
        renderer.renderText("Blue Text", 50.0f, 50.0f, 1.0f, glm::vec3(0.0f, 0.0f, 1.0f));
        
        renderer.renderText("Press ESC to return to menu", size.x - 400.0f, 25.0f, 0.7f, glm::vec3(0.7f, 0.7f, 0.7f));
          // Display FPS (approximately)
        static int frameCount = 0;
        static double lastTime = 0.0;
        static double fps = 0.0;
        
        frameCount++;
        double currentTime = glfwGetTime();
        if (currentTime - lastTime >= 1.0) {
            fps = frameCount / (currentTime - lastTime);
            frameCount = 0;
            lastTime = currentTime;
        }
        
        std::string fpsText = "FPS: " + std::to_string((int)fps);
        renderer.renderText(fpsText, size.x - 150.0f, size.y - 50.0f, 0.8f, glm::vec3(1.0f, 1.0f, 0.0f));
        
        // Add some animated text for demo purposes
        static float animTime = 0.0f;
        animTime += deltaTime;
        
        float pulseScale = 1.0f + 0.3f * sin(animTime * 2.0f);
        renderer.renderTextCentered("✨ ANIMATED TEXT DEMO ✨", size.x / 2.0f, size.y - 300.0f, pulseScale, 
                                   glm::vec3(1.0f, 0.5f + 0.5f * sin(animTime), 1.0f));
        
        // Rainbow colored text line
        std::string rainbowText = "RAINBOW COLORS: ";
        float startX = 50.0f;
        float currentX = startX;
        float yPos = size.y - 500.0f;
        
        renderer.renderText(rainbowText, currentX, yPos, 1.0f, glm::vec3(1.0f, 1.0f, 1.0f));
        currentX += rainbowText.length() * 12.0f; // Approximate character width
        
        std::vector<std::string> colors = {"RED", "ORANGE", "YELLOW", "GREEN", "BLUE", "PURPLE"};
        std::vector<glm::vec3> colorValues = {
            glm::vec3(1.0f, 0.0f, 0.0f), // Red
            glm::vec3(1.0f, 0.5f, 0.0f), // Orange
            glm::vec3(1.0f, 1.0f, 0.0f), // Yellow
            glm::vec3(0.0f, 1.0f, 0.0f), // Green
            glm::vec3(0.0f, 0.5f, 1.0f), // Blue
            glm::vec3(0.7f, 0.0f, 1.0f)  // Purple
        };
        
        for (size_t i = 0; i < colors.size(); ++i) {
            renderer.renderText(colors[i] + " ", currentX, yPos, 1.0f, colorValues[i]);
            currentX += (colors[i].length() + 1) * 12.0f;
        }
    }

    void onDestroy() override {
        renderer.destroy();
        our::clearAllAssets();
    }

    void onImmediateGui() override {
        // You can add ImGui controls here if needed
    }

};
