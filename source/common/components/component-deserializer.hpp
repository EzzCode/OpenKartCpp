#pragma once

#include "../ecs/entity.hpp"
#include "camera.hpp"
#include "mesh-renderer.hpp"
#include "free-camera-controller.hpp"
#include "input.hpp"
#include "movement.hpp"
#include "light.hpp"
#include "rigidbody.hpp"
#include "collider.hpp"
#include "sound.hpp"
#include "race.hpp"
namespace our {

    // Given a json object, this function picks and creates a component in the given entity
    // based on the "type" specified in the json object which is later deserialized from the rest of the json object
    inline void deserializeComponent(const nlohmann::json& data, Entity* entity){
        std::string type = data.value("type", "");
        Component* component = nullptr;
        //TODO: (Req 8) Add an option to deserialize a "MeshRendererComponent" to the following if-else statement
     if(type == CameraComponent::getID()){
            component = entity->addComponent<CameraComponent>();
        } else if (type == FreeCameraControllerComponent::getID()) {
            component = entity->addComponent<FreeCameraControllerComponent>();
        } else if (type == MovementComponent::getID()) {
            component = entity->addComponent<MovementComponent>();
        } else if (type == MeshRendererComponent::getID()) {
            component = entity->addComponent<MeshRendererComponent>();
        } else if (type == LightComponent::getID()) {
            component = entity->addComponent<LightComponent>();
        } else if (type == InputComponent::getID()){
            component = entity->addComponent<InputComponent>();
        }else if (type == RigidbodyComponent::getID()){
            component = entity->addComponent<RigidbodyComponent>();        }else if(type == ColliderComponent::getID()){
            component = entity->addComponent<ColliderComponent>();
        }else if (type == SoundComponent::getID()){
            component = entity->addComponent<SoundComponent>();
        }else if (type == CheckpointComponent::getID()){
            component = entity->addComponent<CheckpointComponent>();
        }else if (type == RacePlayerComponent::getID()){
            component = entity->addComponent<RacePlayerComponent>();
        }else if (type == RaceManagerComponent::getID()){
            component = entity->addComponent<RaceManagerComponent>();
        }

        if(component) component->deserialize(data);
    }

}