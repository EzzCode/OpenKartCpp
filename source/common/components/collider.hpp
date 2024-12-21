#pragma once
#include "../ecs/component.hpp"
#include <glm/glm.hpp>

namespace our
{
    class ColliderComponent : public Component
    {
    public:
        float x_diff, y_diff, z_diff;
        int action;

        static std::string getID() { return "Collider"; }

        void deserialize(const nlohmann::json &data) override;
    };
}