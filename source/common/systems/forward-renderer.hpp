#pragma once
// ADD LIGHT COMPONENT HERE and check line 37
#include "../components/light.hpp"
#include "../ecs/world.hpp"
#include "../components/camera.hpp"
#include "../components/mesh-renderer.hpp"
#include "../asset-loader.hpp"
#include "BulletDebugDrawer.hpp"
// #include "rigidbodySystem.hpp"
#include "../components/rigidbody.hpp"

#include <glad/gl.h>
#include <vector>
#include <algorithm>
#include <map>
#include <string>

// FreeType headers
#include <ft2build.h>
#include FT_FREETYPE_H

namespace our
{
    
    // Structure to hold character information
    struct Character {
        GLuint textureID;  // ID handle of the glyph texture
        glm::ivec2 size;   // Size of glyph
        glm::ivec2 bearing; // Offset from baseline to left/top of glyph
        GLuint advance;    // Offset to advance to next glyph
    };
    
    // The render command stores command that tells the renderer that it should draw
    // the given mesh at the given localToWorld matrix using the given material
    // The renderer will fill this struct using the mesh renderer components
    struct RenderCommand {
        glm::mat4 localToWorld;
        glm::vec3 center;
        Mesh* mesh;
        Material* material;
    };

    // A forward renderer is a renderer that draw the object final color directly to the framebuffer
    // In other words, the fragment shader in the material should output the color that we should see on the screen
    // This is different from more complex renderers that could draw intermediate data to a framebuffer before computing the final color
    // In this project, we only need to implement a forward renderer
    class ForwardRenderer {
        // These window size will be used on multiple occasions (setting the viewport, computing the aspect ratio, etc.)
        glm::ivec2 windowSize;
        // These are two vectors in which we will store the opaque and the transparent commands.
        // We define them here (instead of being local to the "render" function) as an optimization to prevent reallocating them every frame
        std::vector<RenderCommand> opaqueCommands;
        std::vector<RenderCommand> transparentCommands;
        // This vector will store all the light components in the world
        std::vector<LightComponent*> lightCommands;
        // Objects used for rendering a skybox
        Mesh* skySphere;
        TexturedMaterial* skyMaterial;
        btDiscreteDynamicsWorld *dynWorld = nullptr;
        BulletDebugDrawer debugDrawer;
        // Objects used for Postprocessing
        GLuint postprocessFrameBuffer, postProcessVertexArray;
        Texture2D *colorTarget, *depthTarget;
        TexturedMaterial* postprocessMaterial;


        bool debug = false;

        // Text rendering resources
        FT_Library ft;
        FT_Face face;
        std::map<char, Character> characters;
        GLuint textVAO, textVBO;
        ShaderProgram* textShader;
        
        // Helper methods for text rendering
        void initializeTextRendering();
        void loadFont(const std::string& fontPath);
        void destroyTextRendering();
    public:
        // Initialize the renderer including the sky and the Postprocessing objects.
        // windowSize is the width & height of the window (in pixels).
        void setWorld(btDiscreteDynamicsWorld *world){
            dynWorld = world;
        }
        void initialize(glm::ivec2 windowSize, const nlohmann::json& config);
        // Clean up the renderer
        void destroy();
        // This function should be called every frame to draw the given world
        void render(World* world);

        // Text rendering methods
        void renderText(const std::string& text, float x, float y, float scale, const glm::vec3& color);
        void renderTextCentered(const std::string& text, float x, float y, float scale, const glm::vec3& color);

    };

}