#include "material.hpp"

#include "../asset-loader.hpp"
#include "deserialize-utils.hpp"
#include <iostream>

namespace our
{

    // This function should setup the pipeline state and set the shader to be used
    void Material::setup() const
    {
        // TODO: (Req 7) Write this function
        pipelineState.setup();
        shader->use();
    }

    // This function read the material data from a json object
    void Material::deserialize(const nlohmann::json &data)
    {
        if (!data.is_object())
            return;

        if (data.contains("pipelineState"))
        {
            pipelineState.deserialize(data["pipelineState"]);
        }
        shader = AssetLoader<ShaderProgram>::get(data["shader"].get<std::string>());
        transparent = data.value("transparent", false);
    }

    // This function should call the setup of its parent and
    // set the "tint" uniform to the value in the member variable tint
    void TintedMaterial::setup() const
    {
        // TODO: (Req 7) Write this function
        Material::setup();
        shader->set("tint", tint);
    }

    // This function read the material data from a json object
    void TintedMaterial::deserialize(const nlohmann::json &data)
    {
        Material::deserialize(data);
        if (!data.is_object())
            return;
        tint = data.value("tint", glm::vec4(1.0f, 1.0f, 1.0f, 1.0f));
    }

    // This function should call the setup of its parent and
    // set the "alphaThreshold" uniform to the value in the member variable alphaThreshold
    // Then it should bind the texture and sampler to a texture unit and send the unit number to the uniform variable "tex"
    void TexturedMaterial::setup() const
    {
        // TODO: (Req 7) Write this function
        TintedMaterial::setup();
        shader->set("alphaThreshold", alphaThreshold);

        // Bind the texture to unit 0
        glActiveTexture(GL_TEXTURE0);
        texture->bind();
        if (sampler)
        {
            sampler->bind(0);
        }

        // Send the texture unit to the uniform
        shader->set("tex", 0);
    }

    // This function read the material data from a json object
    void TexturedMaterial::deserialize(const nlohmann::json &data)
    {
        TintedMaterial::deserialize(data);
        if (!data.is_object())
            return;
        alphaThreshold = data.value("alphaThreshold", 0.0f);
        texture = AssetLoader<Texture2D>::get(data.value("texture", ""));
        sampler = AssetLoader<Sampler>::get(data.value("sampler", ""));
    }
    //////////////////////////////////////////////////////////////////////
    void LitMaterial::setup() const
    {
        TexturedMaterial::setup();
        if (albedo)
        {
            glActiveTexture(GL_TEXTURE0);
            albedo->bind();
            sampler->bind(0);
            shader->set("material.albedo", 0);
        }
        if (specular)
        {
            glActiveTexture(GL_TEXTURE1);
            specular->bind();
            sampler->bind(1);
            shader->set("material.specular", 1);
        }
        if (roughness)
        {
            glActiveTexture(GL_TEXTURE2);
            roughness->bind();
            sampler->bind(2);
            shader->set("material.roughness", 2);
        }
        if (ambientOcclusion)
        {
            glActiveTexture(GL_TEXTURE3);
            ambientOcclusion->bind();
            sampler->bind(3);
            shader->set("material.ambientOcclusion", 3);
        }
        if (emission)
        {
            glActiveTexture(GL_TEXTURE4);
            emission->bind();
            sampler->bind(4);
            shader->set("material.emission", 4);
        }
        glActiveTexture(GL_TEXTURE0);
    }

    void LitMaterial::deserialize(const nlohmann::json &data)
    {
        TexturedMaterial::deserialize(data);
        if (!data.is_object())
            return;
        albedo = AssetLoader<Texture2D>::get(data.value("albedo", ""));
        specular = AssetLoader<Texture2D>::get(data.value("specular", ""));
        roughness = AssetLoader<Texture2D>::get(data.value("roughness", ""));
        ambientOcclusion = AssetLoader<Texture2D>::get(data.value("ambientOcclusion", ""));
        emission = AssetLoader<Texture2D>::get(data.value("emission", ""));
        sampler = AssetLoader<Sampler>::get(data.value("sampler", ""));
    }

    //////////////////////////////////////////////////////////////////////
    // TextMaterial Implementation
    //////////////////////////////////////////////////////////////////////

    TextMaterial::TextMaterial()
    {
        // Set default text rendering pipeline state
        pipelineState.depthTesting.enabled = false; // Disable depth testing for text (render on top)
        pipelineState.blending.enabled = true;      // Enable blending for transparency
        pipelineState.blending.sourceFactor = GL_SRC_ALPHA;
        pipelineState.blending.destinationFactor = GL_ONE_MINUS_SRC_ALPHA;
        pipelineState.depthMask = false; // Don't write to depth buffer

        textColor = glm::vec3(1.0f, 1.0f, 1.0f); // Default white text
        transparent = true;                      // Text is transparent

        initializeTextRendering();
    }

    TextMaterial::~TextMaterial()
    {
        destroyTextRendering();
    }
    void TextMaterial::setup() const
    {
        Material::setup(); // Set up pipeline state and use shader
        shader->set("textColor", textColor);

        // Set up projection matrix for 2D rendering
        glm::mat4 projection = glm::ortho(0.0f, static_cast<float>(windowSize.x),
                                          0.0f, static_cast<float>(windowSize.y));
        shader->set("projection", projection);

        // Set the texture uniform
        shader->set("text", 0);
    }

    void TextMaterial::initializeTextRendering()
    {
        // Initialize FreeType
        if (FT_Init_FreeType(&ft))
        {
            std::cerr << "ERROR::FREETYPE: Could not init FreeType Library" << std::endl;
            return;
        }

        // Configure VAO/VBO for texture quads
        glGenVertexArrays(1, &textVAO);
        glGenBuffers(1, &textVBO);
        glBindVertexArray(textVAO);
        glBindBuffer(GL_ARRAY_BUFFER, textVBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 6 * 4, NULL, GL_DYNAMIC_DRAW);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), 0);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0);

        // Create text shader
        shader = new ShaderProgram();
        shader->attach("assets/shaders/text.vert", GL_VERTEX_SHADER);
        shader->attach("assets/shaders/text.frag", GL_FRAGMENT_SHADER);
        shader->link();
    }

    bool TextMaterial::loadFont(const std::string &fontPath)
    {
        // Load font face
        if (FT_New_Face(ft, fontPath.c_str(), 0, &face))
        {
            std::cerr << "ERROR::FREETYPE: Failed to load font: " << fontPath << std::endl;
            return false;
        }

        currentFont = fontPath;

        // Set size to load glyphs as
        FT_Set_Pixel_Sizes(face, 0, 48);

        // Disable byte-alignment restriction
        glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

        // Clear existing characters
        for (auto &pair : characters)
        {
            glDeleteTextures(1, &pair.second.textureID);
        }
        characters.clear();

        // Load first 128 characters of ASCII set
        for (unsigned char c = 0; c < 128; c++)
        {
            // Load character glyph
            if (FT_Load_Char(face, c, FT_LOAD_RENDER))
            {
                std::cerr << "ERROR::FREETYPE: Failed to load Glyph" << std::endl;
                continue;
            }

            // Generate texture
            GLuint texture;
            glGenTextures(1, &texture);
            glBindTexture(GL_TEXTURE_2D, texture);
            glTexImage2D(
                GL_TEXTURE_2D,
                0,
                GL_RED,
                face->glyph->bitmap.width,
                face->glyph->bitmap.rows,
                0,
                GL_RED,
                GL_UNSIGNED_BYTE,
                face->glyph->bitmap.buffer);

            // Set texture options
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

            // Store character for later use
            Character character = {
                texture,
                glm::ivec2(face->glyph->bitmap.width, face->glyph->bitmap.rows),
                glm::ivec2(face->glyph->bitmap_left, face->glyph->bitmap_top),
                static_cast<GLuint>(face->glyph->advance.x)};
            characters.insert(std::pair<char, Character>(c, character));
        }
        glBindTexture(GL_TEXTURE_2D, 0);

        return true;
    }
    void TextMaterial::renderText(const std::string &text, float x, float y, float scale, const glm::vec3 &color)
    {
        // Save current OpenGL state
        GLboolean depthTestEnabled = glIsEnabled(GL_DEPTH_TEST);
        GLboolean blendEnabled = glIsEnabled(GL_BLEND);
        GLint blendSrcRGB, blendDstRGB, blendSrcAlpha, blendDstAlpha;
        glGetIntegerv(GL_BLEND_SRC_RGB, &blendSrcRGB);
        glGetIntegerv(GL_BLEND_DST_RGB, &blendDstRGB);
        glGetIntegerv(GL_BLEND_SRC_ALPHA, &blendSrcAlpha);
        glGetIntegerv(GL_BLEND_DST_ALPHA, &blendDstAlpha);

        GLint currentProgram, currentVAO, currentTexture;
        glGetIntegerv(GL_CURRENT_PROGRAM, &currentProgram);
        glGetIntegerv(GL_VERTEX_ARRAY_BINDING, &currentVAO);
        glGetIntegerv(GL_TEXTURE_BINDING_2D, &currentTexture);

        // Set up text rendering state
        glDisable(GL_DEPTH_TEST); // Disable depth testing for text (render on top)
        glEnable(GL_BLEND);       // Enable blending for transparency
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        // Use shader and set uniforms
        shader->use();
        shader->set("textColor", color);

        // Set up projection matrix for 2D rendering
        glm::mat4 projection = glm::ortho(0.0f, static_cast<float>(windowSize.x),
                                          0.0f, static_cast<float>(windowSize.y));
        shader->set("projection", projection);

        glActiveTexture(GL_TEXTURE0);
        glBindVertexArray(textVAO);

        // Iterate through all characters
        for (auto c = text.begin(); c != text.end(); c++)
        {
            Character ch = characters[*c];

            float xpos = x + ch.bearing.x * scale;
            float ypos = y - (ch.size.y - ch.bearing.y) * scale;

            float w = ch.size.x * scale;
            float h = ch.size.y * scale;

            // Update VBO for each character
            float vertices[6][4] = {
                {xpos, ypos + h, 0.0f, 0.0f},
                {xpos, ypos, 0.0f, 1.0f},
                {xpos + w, ypos, 1.0f, 1.0f},

                {xpos, ypos + h, 0.0f, 0.0f},
                {xpos + w, ypos, 1.0f, 1.0f},
                {xpos + w, ypos + h, 1.0f, 0.0f}};

            // Render glyph texture over quad
            glBindTexture(GL_TEXTURE_2D, ch.textureID);

            // Update content of VBO memory
            glBindBuffer(GL_ARRAY_BUFFER, textVBO);
            glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);
            glBindBuffer(GL_ARRAY_BUFFER, 0);

            // Render quad
            glDrawArrays(GL_TRIANGLES, 0, 6);

            // Advance cursors for next glyph
            x += (ch.advance >> 6) * scale;
        }
        glBindVertexArray(currentVAO);
        glBindTexture(GL_TEXTURE_2D, currentTexture);

        // Restore previous OpenGL state
        if (!blendEnabled)
            glDisable(GL_BLEND);
        else
            glBlendFuncSeparate(blendSrcRGB, blendDstRGB, blendSrcAlpha, blendDstAlpha);
        if (depthTestEnabled)
            glEnable(GL_DEPTH_TEST);

        // Always restore the previous program, even if it was 0 (unbound)
        glUseProgram(currentProgram);
    }

    void TextMaterial::renderTextCentered(const std::string &text, float x, float y, float scale, const glm::vec3 &color)
    {
        // Calculate text width
        float textWidth = 0;
        for (char c : text)
        {
            Character ch = characters[c];
            textWidth += (ch.advance >> 6) * scale;
        }

        // Render text centered
        renderText(text, x - textWidth / 2.0f, y, scale, color);
    }

    void TextMaterial::destroyTextRendering()
    {
        // Clean up character textures
        for (auto &pair : characters)
        {
            glDeleteTextures(1, &pair.second.textureID);
        }
        characters.clear();

        // Clean up FreeType
        if (face)
            FT_Done_Face(face);
        if (ft)
            FT_Done_FreeType(ft);

        // Clean up OpenGL resources
        glDeleteVertexArrays(1, &textVAO);
        glDeleteBuffers(1, &textVBO);
        if (shader)
        {
            delete shader;
            shader = nullptr;
        }
    }

}