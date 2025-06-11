#pragma once

#include "../ecs/world.hpp"
#include "../components/text-renderer.hpp"
#include "../material/material.hpp"
#include <glm/glm.hpp>

namespace our
{

    // This system handles rendering text for all entities with TextRendererComponent
    // It uses a TextMaterial to perform the actual rendering
    class TextRenderSystem
    {
    private:
        TextMaterial *textMaterial = nullptr;
        glm::ivec2 windowSize;

    public:
        void initialize(const glm::ivec2 &windowSize);
        void destroy();
        void render(World *world);
        void setWindowSize(const glm::ivec2 &size);
    };

}
