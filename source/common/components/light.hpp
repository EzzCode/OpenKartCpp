#pragma once
#include "../ecs/component.hpp"
#include "../shader/shader.hpp"
#include <glm/glm.hpp>
namespace our
{  
    enum class lightType
    {
        DIRECTIONAL,
        POINT,
        SPOT
    };
    class LightComponent : public Component
    {
        public:
            lightType lightType;
            glm::vec3 direction = glm::vec3(0.0f, 0.0f, -1.0f);

            glm::vec3 color = glm::vec3(1.0f, 1.0f, 1.0f);
            glm::vec3 attenuation = glm::vec3(1.0f, 0.0f, 0.0f);
            float inner_cone_angle = 0.0f;
            float outer_cone_angle = 0.0f;
        // The ID of this component type is "Light"
        static const std::string& getID() { static const std::string id = "Light"; return id; }
        // Reads light parameters from the given json object
        void deserialize(const nlohmann::json& data) override;
    };
} // namespace our