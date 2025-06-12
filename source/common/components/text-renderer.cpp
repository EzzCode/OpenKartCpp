#include "text-renderer.hpp"
#include "../deserialize-utils.hpp"

namespace our {

    void TextRendererComponent::deserialize(const nlohmann::json& data) {
        if (!data.is_object()) return;
        
        // Use safe value extraction with defaults
        text = data.value("text", std::string(""));
        scale = std::max(0.1f, data.value("scale", 1.0f)); // Ensure minimum scale
        color = data.value("color", glm::vec3(1.0f, 1.0f, 1.0f));
        centered = data.value("centered", false);
        visible = data.value("visible", true);
        
        // Validate color components are in valid range
        color.r = std::clamp(color.r, 0.0f, 1.0f);
        color.g = std::clamp(color.g, 0.0f, 1.0f);
        color.b = std::clamp(color.b, 0.0f, 1.0f);
    }

}
