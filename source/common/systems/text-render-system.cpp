#include "text-render-system.hpp"
#include "../components/text-renderer.hpp"
#include <iostream>

namespace our
{

    void TextRenderSystem::initialize(const glm::ivec2 &windowSize)
    {
        this->windowSize = windowSize;

        // Create and initialize text material
        textMaterial = new TextMaterial();
        textMaterial->setWindowSize(windowSize);
        textMaterial->loadFont("assets/fonts/arial.ttf");
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

        // Ensure we're rendering to the default framebuffer for HUD elements
        glBindFramebuffer(GL_FRAMEBUFFER, 0);

        // Iterate through all entities and render text for those with TextRendererComponent
        for (auto entity : world->getEntities())
        {
            auto textRenderer = entity->getComponent<TextRendererComponent>();
            if (textRenderer && textRenderer->visible && !textRenderer->text.empty())
            {

                // Get the entity's world position
                glm::mat4 worldMatrix = entity->getLocalToWorldMatrix();
                glm::vec3 worldPosition = glm::vec3(worldMatrix[3]);
                (void)worldPosition; // Suppress unused variable warning - may be used in future

                // Convert world position to screen coordinates (for HUD, we can use the transform directly)
                // For text rendering, we typically want screen-space coordinates
                float x = entity->localTransform.position.x;
                float y = entity->localTransform.position.y;

                // Apply any rotation scale from the entity's transform to the text scale
                float finalScale = textRenderer->scale * entity->localTransform.scale.x;

                // Render the text
                if (textRenderer->centered)
                {
                    textMaterial->renderTextCentered(textRenderer->text, x, y, finalScale, textRenderer->color);
                }
                else
                {
                    textMaterial->renderText(textRenderer->text, x, y, finalScale, textRenderer->color);
                }
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
