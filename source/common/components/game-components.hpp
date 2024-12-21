#pragma once
#include "../ecs/component.hpp"

namespace our {
    class CoinComponent : public Component {
    public:
        int value = 1;
        // The ID of this component type is "Coin"
        static std::string getID() { return "Coin"; }
         void deserialize(const nlohmann::json &data){
                value= data.value("value",value);
            }
    };

    class ObstacleComponent : public Component {
    public:
        // The ID of this component type is "Obstacle"
        static std::string getID() { return "Obstacle"; }
        void deserialize(const nlohmann::json &data) {

        }
    };

}