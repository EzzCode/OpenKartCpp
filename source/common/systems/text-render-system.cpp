#include "text-render-system.hpp"
#include "../components/text-renderer.hpp"
#include <iostream>

namespace our
{

    void TextRenderSystem::initialize(const glm::ivec2 &windowSize)
    {
        this->windowSize = windowSize;

        // Clean up existing material
        if (textMaterial)
        {
            delete textMaterial;
        }

        textMaterial = new TextMaterial();
        textMaterial->setWindowSize(windowSize);

        if (!textMaterial->loadFont("assets/fonts/arial.ttf"))
        {
            std::cerr << "WARNING: Failed to load default font" << std::endl;
        }
    }

    void TextRenderSystem::destroy()
    {
        if (textMaterial)
        {
            delete textMaterial;
            textMaterial = nullptr;
        }
    }

    void TextRenderSystem::render(World *world)
    {
        if (!textMaterial || !world)
            return;

        // Ensure we're rendering to the default framebuffer
        glBindFramebuffer(GL_FRAMEBUFFER, 0);

        // Get all entities with TextRendererComponent
        auto entities = world->getEntities();

        for (auto entity : entities)
        {
            if (!entity)
                continue;

            auto textRenderer = entity->getComponent<TextRendererComponent>();
            if (!textRenderer || !textRenderer->visible || textRenderer->text.empty())
                continue;

            // Use screen-space coordinates from entity transform
            float x = entity->localTransform.position.x;
            float y = entity->localTransform.position.y;
            float finalScale = textRenderer->scale * entity->localTransform.scale.x;

            try
            {
                if (textRenderer->centered)
                {
                    textMaterial->renderTextCentered(textRenderer->text, x, y, finalScale, textRenderer->color);
                }
                else
                {
                    textMaterial->renderText(textRenderer->text, x, y, finalScale, textRenderer->color);
                }
            }
            catch (const std::exception &e)
            {
                std::cerr << "Error rendering text: " << e.what() << std::endl;
            }
        }
    }

    void TextRenderSystem::setWindowSize(const glm::ivec2 &size)
    {
        windowSize = size;
        if (textMaterial)
        {
            textMaterial->setWindowSize(size);
        }
    }

}
