#pragma once

#include "../ecs/component.hpp"
#include "../material/material.hpp"
#include <glm/glm.hpp>
#include <string>

namespace our {

    // This component denotes that the entity should render text at its transform position
    // The text renderer system will collect all entities with this component and render their text
    class TextRendererComponent : public Component {
    public:
        std::string text;           // The text to render
        float scale = 1.0f;         // Scale factor for the text
        glm::vec3 color = glm::vec3(1.0f, 1.0f, 1.0f); // Text color (default white)
        bool centered = false;      // Whether to center the text at the position
        bool visible = true;        // Whether the text is visible
        
        // The ID of this component type is "Text Renderer"
        static std::string getID() { return "Text Renderer"; }

        // Reads text properties from the json object
        void deserialize(const nlohmann::json& data) override;
    };

}
