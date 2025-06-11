#pragma once

#include "pipeline-state.hpp"
#include "../texture/texture2d.hpp"
#include "../texture/sampler.hpp"
#include "../shader/shader.hpp"

#include <glm/vec4.hpp>
#include <json/json.hpp>

// FreeType headers for text rendering
#include <ft2build.h>
#include FT_FREETYPE_H
#include <map>
#include <string>

namespace our
{

    // Structure to hold character information for text rendering
    struct Character
    {
        GLuint textureID;   // ID handle of the glyph texture
        glm::ivec2 size;    // Size of glyph
        glm::ivec2 bearing; // Offset from baseline to left/top of glyph
        GLuint advance;     // Offset to advance to next glyph
    };

    // This is the base class for all the materials
    // It contains the 3 essential components required by any material
    // 1- The pipeline state when drawing objects using this material
    // 2- The shader program used to draw objects using this material
    // 3- Whether this material is transparent or not
    // Materials that send uniforms to the shader should inherit from the is material and add the required uniforms
    class Material
    {
    public:
        PipelineState pipelineState;
        ShaderProgram *shader;
        bool transparent;

        // Virtual destructor to ensure proper cleanup of derived classes
        virtual ~Material() = default;

        // This function does 2 things: setup the pipeline state and set the shader program to be used
        virtual void setup() const;
        // This function read a material from a json object
        virtual void deserialize(const nlohmann::json &data);
    };

    // This material adds a uniform for a tint (a color that will be sent to the shader)
    // An example where this material can be used is when the whole object has only color which defined by tint
    class TintedMaterial : public Material
    {
    public:
        glm::vec4 tint;

        void setup() const override;
        void deserialize(const nlohmann::json &data) override;
    };

    // This material adds two uniforms (besides the tint from Tinted Material)
    // The uniforms are:
    // - "tex" which is a Sampler2D. "texture" and "sampler" will be bound to it.
    // - "alphaThreshold" which defined the alpha limit below which the pixel should be discarded
    // An example where this material can be used is when the object has a texture
    class TexturedMaterial : public TintedMaterial
    {
    public:
        Texture2D *texture;
        Sampler *sampler;
        float alphaThreshold;

        void setup() const override;
        void deserialize(const nlohmann::json &data) override;
    };
    // This material adds 4 more textures to the TexturedMaterial
    // The textures are:
    // - "albedo" which is the color of the object (how much light is absorbed)
    // - "specular" which is the color of the specular highlights of the object (how much light is reflected)
    // - "roughness" which is the roughness of the object (how much light is scattered)
    // - "ambientOcclusion" which is (how much light is blocked from the environment)
    // - "emissions" which is the color of the light emitted by the object (how much light is emitted)
    class LitMaterial : public TexturedMaterial
    {
    public:
        Texture2D *albedo;
        Texture2D *specular;
        Texture2D *roughness;
        Texture2D *ambientOcclusion;
        Texture2D *emission;

        void setup() const override;
        void deserialize(const nlohmann::json &data) override;
    };

    // Text material for rendering text using FreeType generated character textures
    // This material handles text rendering with configurable color and font management
    class TextMaterial : public Material
    {
    public:
        glm::vec3 textColor;
        std::map<char, Character> characters;
        GLuint textVAO, textVBO;
        FT_Library ft;
        FT_Face face;
        glm::ivec2 windowSize;
        std::string currentFont;

        TextMaterial();
        ~TextMaterial();

        void setup() const override;
        bool loadFont(const std::string &fontPath);
        void renderText(const std::string &text, float x, float y, float scale, const glm::vec3 &color);
        void renderTextCentered(const std::string &text, float x, float y, float scale, const glm::vec3 &color);
        void setWindowSize(const glm::ivec2 &size) { windowSize = size; }

    private:
        void initializeTextRendering();
        void destroyTextRendering();
    };

    // This function returns a new material instance based on the given type
    inline Material *createMaterialFromType(const std::string &type)
    {
        if (type == "tinted")
        {
            return new TintedMaterial();
        }
        else if (type == "textured")
        {
            return new TexturedMaterial();
        }
        else if (type == "lit")
        {
            return new LitMaterial();
        }
        else if (type == "text")
        {
            return new TextMaterial();
        }
        else
        {
            return new Material();
        }
    }

}