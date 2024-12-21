#include "light.hpp"
#include "../ecs/entity.hpp"
#include "../deserialize-utils.hpp"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <asset-loader.hpp>

namespace our {
    // read the light data from the json object
    void LightComponent::deserialize(const nlohmann::json& data) {
        if (!data.is_object()) return;

        std::string lightTypestr = data.value("lightType", "Directional");

        if (lightTypestr == "Directional") {
            lightType = LightType::DIRECTIONAL;
        }
        else if (lightTypestr == "Point") {
            lightType = LightType::POINT;
        }
        else if (lightTypestr == "Spot") {
            lightType = LightType::SPOT;
        }

        // was 15 and 30
        color = data.value("color", glm::vec3(1.0f, 1.0f, 1.0f));
        cone_angles = data.value("cone_angles", glm::vec2(glm::radians(15.0f), glm::radians(30.0f)));
        attenuation = data.value("attenuation", glm::vec3(0.0f, 0.0f, 1.0f));
        direction = data.value("direction", glm::vec3(-1.0f,0.0f, 0.0f));

    }

}