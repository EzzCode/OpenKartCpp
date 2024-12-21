#pragma once
#include "../ecs/component.hpp"
#include "../shader/shader.hpp"
#include <glm/mat4x4.hpp>
#include <glm/glm.hpp>

namespace our
{
  enum class LightType
  {
    DIRECTIONAL,
    POINT,
    SPOT
  };

  class LightComponent : public Component
  {
  public:
    LightType lightType;
    glm::vec3 color = glm::vec3(0, 0, 0);

    glm::vec3 attenuation = glm::vec3(0.0f, 0.0f, 0.0f);
    glm::vec3 direction = glm::vec3(0.0f, 0.0f, 0.0f);
    glm::vec2 cone_angles = glm::vec2(0.0f, 0.0f);

    static std::string getID() { return "Light"; }

    // read the light data from the json object
    void deserialize(const nlohmann::json& data) override;
  };
}