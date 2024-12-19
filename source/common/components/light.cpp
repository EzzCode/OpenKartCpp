#include "light.hpp"
#include "../deserialize-utils.hpp"
namespace our
{  
    // Reads light parameters from the given json object
    void LightComponent::deserialize(const nlohmann::json& data)
    {
        if (!data.is_object())
            return;
        std::string typeStr = data.value("lightType", "directional");
        std::transform(typeStr.begin(), typeStr.end(), typeStr.begin(), ::tolower);
        if (typeStr == "directional")
        {
            lightType = Type::DIRECTIONAL;
        }
        else if (typeStr == "point")
        {
            lightType = lightType::POINT;
        }
        else if (typeStr == "spot")
        {
            lightType = lightType::SPOT;
        }
        attenuation = data.value("attenuation", glm::vec3(-1.0f, 0.0f, 0.0f));
        direction = data.value("direction", glm::vec3(-1.0f,0.0f, 0.0f));
        inner_cone_angle = data.value("innerConeAngle", glm::radians(15.0f));
        outer_cone_angle = data.value("outerConeAngle", glm::radians(30.0f));  
        color = data.value("color", glm::vec3(1.0f, 0.0f, 1.0f));
    }
} // namespace our