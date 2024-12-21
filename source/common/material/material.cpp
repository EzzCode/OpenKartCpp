#include "material.hpp"

#include "../asset-loader.hpp"
#include "deserialize-utils.hpp"

namespace our {

    // This function should setup the pipeline state and set the shader to be used
    void Material::setup() const {
        //TODO: (Req 7) Write this function

        // setup piplineState
        pipelineState.setup();

        // set shader to be used
        shader->use();
    }

    // This function read the material data from a json object
    void Material::deserialize(const nlohmann::json& data){
        if(!data.is_object()) return;

        if(data.contains("pipelineState")){
            pipelineState.deserialize(data["pipelineState"]);
        }
        shader = AssetLoader<ShaderProgram>::get(data["shader"].get<std::string>());
        transparent = data.value("transparent", false);
    }

    // This function should call the setup of its parent and
    // set the "tint" uniform to the value in the member variable tint 
    void TintedMaterial::setup() const {
        //TODO: (Req 7) Write this function
        
        // call the setup of the parent
        Material::setup();

        // set "tint" uniform with tint
        shader->set("tint", tint); 
    }

    // This function read the material data from a json object
    void TintedMaterial::deserialize(const nlohmann::json& data){
        Material::deserialize(data);
        if(!data.is_object()) return;
        tint = data.value("tint", glm::vec4(1.0f, 1.0f, 1.0f, 1.0f));
    }

    // This function should call the setup of its parent and
    // set the "alphaThreshold" uniform to the value in the member variable alphaThreshold
    // Then it should bind the texture and sampler to a texture unit and send the unit number to the uniform variable "tex" 
    void TexturedMaterial::setup() const {
        //TODO: (Req 7) Write this function

        // call the setup of the parent
        TintedMaterial::setup();

        // set "alphaThreshold" uniform with alphaThreshold
        shader->set("alphaThreshold", alphaThreshold);

    // call glactivetexture to ge the active texture
        
        // bind the texture
        texture->bind();

        if(sampler != nullptr) {
            //bind the sampler to 0
            sampler->bind(0);
        }
        

        // set "tex" uniform with 0
        shader->set("tex", 0);
    }

    // This function read the material data from a json object
    void TexturedMaterial::deserialize(const nlohmann::json& data){
        TintedMaterial::deserialize(data);
        if(!data.is_object()) return;
        alphaThreshold = data.value("alphaThreshold", 0.0f);
        texture = AssetLoader<Texture2D>::get(data.value("texture", ""));
        sampler = AssetLoader<Sampler>::get(data.value("sampler", ""));
    }


    // !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
    void LitMaterial::setup() const {
        TexturedMaterial::setup();

        if(albedo != nullptr)
        {
            // select Texture unit to be active
            glActiveTexture(GL_TEXTURE0);
            // bind the texture to the active texture unit
            albedo->bind();
            // bind the sampler to the active texture unit
            sampler->bind(0);
            shader->set("material.albedo", 0);
        }

        if (specular != nullptr)
        {
            // select Texture unit to be active
            glActiveTexture(GL_TEXTURE1);
            // bind the texture to the active texture unit
            specular->bind();
            // bind the sampler to the active texture unit
            sampler->bind(1);
            shader->set("material.specular", 1);
        }

        if (roughness != nullptr)
        {
            // select Texture unit to be active
            glActiveTexture(GL_TEXTURE2);
            // bind the texture to the active texture unit
            roughness->bind();
            // bind the sampler to the active texture unit
            sampler->bind(2);
            shader->set("material.roughness", 2);
        }

        if(ao != nullptr){
            // select Texture unit to be active
            glActiveTexture(GL_TEXTURE3);
            // bind the texture to the active texture unit
            ao->bind();
            // bind the sampler to the active texture unit
            sampler->bind(3);
            shader->set("material.ambient_occlusion", 3);
        }

        if(emission != nullptr)
        {
            // select Texture unit to be active
            glActiveTexture(GL_TEXTURE4);
            // bind the texture to the active texture unit
            emission->bind();
            // bind the sampler to the active texture unit
            sampler->bind(4);
            shader->set("material.emission", 4);
        }
        glActiveTexture(GL_TEXTURE0);
    }

    void LitMaterial::deserialize(const nlohmann::json &data) {
        TexturedMaterial::deserialize(data);
        if (!data.is_object())
            return;
        
        albedo = AssetLoader<Texture2D>::get(data.value("albedo", ""));
        specular = AssetLoader<Texture2D>::get(data.value("specular", ""));
        roughness = AssetLoader<Texture2D>::get(data.value("roughness", ""));
        emission = AssetLoader<Texture2D>::get(data.value("emission", ""));
        ao = AssetLoader<Texture2D>::get(data.value("ao", ""));
        sampler = AssetLoader<Sampler>::get(data.value("sampler", ""));
    }

}