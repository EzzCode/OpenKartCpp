#include "collider.hpp"
#include "../ecs/entity.hpp"
#include "../deserialize-utils.hpp"

namespace our {
    void ColliderComponent::deserialize(const nlohmann::json &data)
    {
        if (!data.is_object())
            return;
        x_diff = data.value("x_diff", x_diff);
        y_diff = data.value("y_diff", y_diff);
        z_diff = data.value("z_diff", z_diff);
        action = data.value("action", action);
    }
}