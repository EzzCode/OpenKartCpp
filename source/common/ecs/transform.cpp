#include "entity.hpp"
#include "../deserialize-utils.hpp"

#include <glm/gtx/euler_angles.hpp>

namespace our {

    // This function computes and returns a matrix that represents this transform
    // Remember that the order of transformations is: Scaling, Rotation then Translation
    // HINT: to convert euler angles to a rotation matrix, you can use glm::yawPitchRoll
    glm::mat4 Transform::toMat4() const {
        //TODO: (Req 3) Write this function

        //return glm::mat4(1.0f); 
        // create transformation matrix
        glm::mat4 transformation_matrix(1.0f);

        // apply scaling
        transformation_matrix = glm::scale(glm::mat4(1.0f), scale) * transformation_matrix;

        //apply rotation
        transformation_matrix = glm::yawPitchRoll(rotation.y, rotation.x, rotation.z) * transformation_matrix;

        // apply translation
        transformation_matrix = glm::translate(glm::mat4(1.0f), position) * transformation_matrix;

        return transformation_matrix; 

    }

     // Deserializes the entity data and components from a json object
    void Transform::deserialize(const nlohmann::json& data){
        position = data.value("position", position);
        rotation = glm::radians(data.value("rotation", glm::degrees(rotation)));
        scale    = data.value("scale", scale);
    }

}