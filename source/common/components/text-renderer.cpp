#include "text-renderer.hpp"
#include "../deserialize-utils.hpp"

namespace our {

    void TextRendererComponent::deserialize(const nlohmann::json& data) {
        if (!data.is_object()) return;
        
        text = data.value("text", text);
        scale = data.value("scale", scale);
        color = data.value("color", color);
        centered = data.value("centered", centered);
        visible = data.value("visible", visible);
    }

}
