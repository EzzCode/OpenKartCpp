#include "forward-renderer.hpp"
#include "../mesh/mesh-utils.hpp"
#include "../texture/texture-utils.hpp"
#include <iostream>

namespace our
{
    void ForwardRenderer::initialize(glm::ivec2 windowSize, const nlohmann::json &config)
    {
        // First, we store the window size for later use
        this->windowSize = windowSize;

        // Read debug configuration
        debug = config.value("debug", false);

        // Then we check if there is a sky texture in the configuration
        if (config.contains("sky"))
        {
            // First, we create a sphere which will be used to draw the sky
            this->skySphere = mesh_utils::sphere(glm::ivec2(16, 16));

            // We can draw the sky using the same shader used to draw textured objects
            ShaderProgram *skyShader = new ShaderProgram();
            skyShader->attach("assets/shaders/textured.vert", GL_VERTEX_SHADER);
            skyShader->attach("assets/shaders/textured.frag", GL_FRAGMENT_SHADER);
            skyShader->link();

            // TODO: (Req 10) Pick the correct pipeline state to draw the sky
            //  Hints: the sky will be draw after the opaque objects so we would need depth testing but which depth funtion should we pick?
            //  We will draw the sphere from the inside, so what options should we pick for the face culling.
            PipelineState skyPipelineState{};
            // enable depth testing
            skyPipelineState.depthTesting.enabled = true;

            // enable less than or equal function (we will make it the furthest)
            skyPipelineState.depthTesting.function = GL_LEQUAL;

            // enable face culling
            skyPipelineState.faceCulling.enabled = true;

            // cull the front face (we want the inside)
            skyPipelineState.faceCulling.culledFace = GL_FRONT;
            // Load the sky texture (note that we don't need mipmaps since we want to avoid any unnecessary blurring while rendering the sky)
            std::string skyTextureFile = config.value<std::string>("sky", "");
            Texture2D *skyTexture = texture_utils::loadImage(skyTextureFile, false);

            // Setup a sampler for the sky
            Sampler *skySampler = new Sampler();
            skySampler->set(GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            skySampler->set(GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            skySampler->set(GL_TEXTURE_WRAP_S, GL_REPEAT);
            skySampler->set(GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

            // Combine all the aforementioned objects (except the mesh) into a material
            this->skyMaterial = new TexturedMaterial();
            this->skyMaterial->shader = skyShader;
            this->skyMaterial->texture = skyTexture;
            this->skyMaterial->sampler = skySampler;
            this->skyMaterial->pipelineState = skyPipelineState;
            this->skyMaterial->tint = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
            this->skyMaterial->alphaThreshold = 1.0f;
            this->skyMaterial->transparent = false;
        }

        // Then we check if there is a postprocessing shader in the configuration
        if (config.contains("postprocess"))
        {
            // TODO: (Req 11) Create a framebuffer
            glGenFramebuffers(1, &postprocessFrameBuffer);
            glBindFramebuffer(GL_FRAMEBUFFER, postprocessFrameBuffer);

            // TODO: (Req 11) Create a color and a depth texture and attach them to the framebuffer
            //  Hints: The color format can be (Red, Green, Blue and Alpha components with 8 bits for each channel).
            //  The depth format can be (Depth component with 24 bits).
            colorTarget = texture_utils::empty(GL_RGBA8, windowSize);
            depthTarget = texture_utils::empty(GL_DEPTH_COMPONENT24, windowSize);
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, colorTarget->getOpenGLName(), 0);
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthTarget->getOpenGLName(), 0);

            // TODO: (Req 11) Unbind the framebuffer just to be safe
            glBindFramebuffer(GL_FRAMEBUFFER, 0);

            // Create a vertex array to use for drawing the texture
            glGenVertexArrays(1, &postProcessVertexArray);

            // Create a sampler to use for sampling the scene texture in the post processing shader
            Sampler *postprocessSampler = new Sampler();
            postprocessSampler->set(GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            postprocessSampler->set(GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            postprocessSampler->set(GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            postprocessSampler->set(GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

            // Create the post processing shader
            ShaderProgram *postprocessShader = new ShaderProgram();
            postprocessShader->attach("assets/shaders/fullscreen.vert", GL_VERTEX_SHADER);
            postprocessShader->attach(config.value<std::string>("postprocess", ""), GL_FRAGMENT_SHADER);
            postprocessShader->link();

            // Create a post processing material
            postprocessMaterial = new TexturedMaterial();
            postprocessMaterial->shader = postprocessShader;
            postprocessMaterial->texture = colorTarget;
            postprocessMaterial->sampler = postprocessSampler;
            // The default options are fine but we don't need to interact with the depth buffer
            // so it is more performant to disable the depth mask
            postprocessMaterial->pipelineState.depthMask = false;
        }
        if (debug == true)
        {
            debugDrawer.setDebugMode(btIDebugDraw::DBG_DrawWireframe);
            dynWorld->setDebugDrawer(&debugDrawer);
            debugDrawer.glfw3_device_create();
        }

        // Initialize FreeType
        if (FT_Init_FreeType(&ft))
        {
            std::cerr << "ERROR::FREETYPE: Could not init FreeType Library" << std::endl;
            return;
        }

        // Load font
        loadFont("assets/fonts/arial.ttf");

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

        // Load text shader
        textShader = new ShaderProgram();
        textShader->attach("assets/shaders/text.vert", GL_VERTEX_SHADER);
        textShader->attach("assets/shaders/text.frag", GL_FRAGMENT_SHADER);
        textShader->link();
    }

    void ForwardRenderer::destroy()
    {
        if (debug)
            debugDrawer.glfw3_device_destroy();
        // Delete all objects related to the sky
        if (skyMaterial)
        {
            delete skySphere;
            delete skyMaterial->shader;
            delete skyMaterial->texture;
            delete skyMaterial->sampler;
            delete skyMaterial;
        }
        // Delete all objects related to post processing
        if (postprocessMaterial)
        {
            glDeleteFramebuffers(1, &postprocessFrameBuffer);
            glDeleteVertexArrays(1, &postProcessVertexArray);
            delete colorTarget;
            delete depthTarget;
            delete postprocessMaterial->sampler;
            delete postprocessMaterial->shader;
            delete postprocessMaterial;
        }

        // Clean up character textures
        for (auto &pair : characters)
        {
            glDeleteTextures(1, &pair.second.textureID);
        }
        characters.clear();

        // Clean up FreeType
        FT_Done_Face(face);
        FT_Done_FreeType(ft);

        // Clean up OpenGL resources
        glDeleteVertexArrays(1, &textVAO);
        glDeleteBuffers(1, &textVBO);
        if (textShader)
        {
            delete textShader;
            textShader = nullptr;
        }
    }

    void ForwardRenderer::render(World *world)
    {
        // First of all, we search for a camera and for all the mesh renderers
        CameraComponent *camera = nullptr;
        opaqueCommands.clear();
        transparentCommands.clear();
        lightCommands.clear();
        for (auto entity : world->getEntities())
        {
            // If we hadn't found a camera yet, we look for a camera in this entity
            if (!camera)
                camera = entity->getComponent<CameraComponent>();
            // If this entity has a mesh renderer component
            if (auto meshRenderer = entity->getComponent<MeshRendererComponent>(); meshRenderer)
            {
                // We construct a command from it
                RenderCommand command;
                command.localToWorld = meshRenderer->getOwner()->getLocalToWorldMatrix();
                command.center = glm::vec3(command.localToWorld * glm::vec4(0, 0, 0, 1));
                command.mesh = meshRenderer->mesh;
                command.material = meshRenderer->material;
                // if it is transparent, we add it to the transparent commands list
                if (command.material->transparent)
                {
                    transparentCommands.push_back(command);
                }
                else
                {
                    // Otherwise, we add it to the opaque command list
                    opaqueCommands.push_back(command);
                }
            }
            // Add LightComponent to the entity
            if (auto light = entity->getComponent<LightComponent>(); light)
            {
                lightCommands.push_back(light);
            }
        }

        // If there is no camera, we return (we cannot render without a camera)
        if (camera == nullptr)
            return;

        // TODO: (Req 9) Modify the following line such that "cameraForward" contains a vector pointing the camera forward direction
        //  HINT: See how you wrote the CameraComponent::getViewMatrix, it should help you solve this one
        auto M = camera->getOwner()->getLocalToWorldMatrix();
        glm::vec3 eye = M * glm::vec4(0, 0, 0, 1);
        glm::vec3 center = M * glm::vec4(0, 0, -1, 1);
        glm::vec3 cameraForward = glm::normalize(center - eye);

        std::sort(transparentCommands.begin(), transparentCommands.end(), [cameraForward](const RenderCommand &first, const RenderCommand &second)
                  {
                      // TODO: (Req 9) Finish this function
                      // HINT: the following return should return true "first" should be drawn before "second". 
                      return glm::dot(first.center, cameraForward) > glm::dot(second.center, cameraForward); });

        // TODO: (Req 9) Get the camera ViewProjection matrix and store it in VP
        glm::mat4 VP = camera->getProjectionMatrix(windowSize) * camera->getViewMatrix();

        // TODO: (Req 9) Set the OpenGL viewport using viewportStart and viewportSize
        glViewport(0, 0, windowSize.x, windowSize.y);

        // TODO: (Req 9) Set the clear color to black and the clear depth to 1
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClearDepth(1.0f);

        // TODO: (Req 9) Set the color mask to true and the depth mask to true (to ensure the glClear will affect the framebuffer)
        glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
        glDepthMask(GL_TRUE);

        // If there is a postprocess material, bind the framebuffer
        if (postprocessMaterial)
        {
            // TODO: (Req 11) bind the framebuffer
            glBindFramebuffer(GL_FRAMEBUFFER, postprocessFrameBuffer);
        }

        // TODO: (Req 9) Clear the color and depth buffers
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // TODO: (Req 9) Draw all the opaque commands
        //  Don't forget to set the "transform" uniform to be equal the model-view-projection matrix for each render command
        for (const auto &command : opaqueCommands)
        {
            command.material->setup();
            command.material->shader->set("transform", VP * command.localToWorld);

            /////////////////////////// ADD LIGHT COMPONENT HERE ///////////////////////////
            if (dynamic_cast<LitMaterial *>(command.material))
            {
                LitMaterial *litMaterial = dynamic_cast<LitMaterial *>(command.material);
                command.material->shader->set("camera_position", eye);
                command.material->shader->set("light_count", (int)lightCommands.size());
                command.material->shader->set("VP", VP);
                command.material->shader->set("M", command.localToWorld);
                command.material->shader->set("M_IT", glm::transpose(glm::inverse(command.localToWorld)));
                // command.material->shader->set("M_IT", glm::inverse(command.localToWorld));

                for (size_t i = 0; i < lightCommands.size(); i++)
                {
                    LightComponent *light = lightCommands[i];
                    glm::vec3 light_position;
                    std::string light_name = "lights[" + std::to_string(i) + "]";

                    if (light->getOwner()->parent)
                    {
                        light_position = light->getOwner()->parent->localTransform.position + light->getOwner()->localTransform.position;
                    }
                    else
                    {
                        light_position = light->getOwner()->localTransform.position;
                    }

                    if (light->lightType == lightType::DIRECTIONAL)
                    {
                        command.material->shader->set(light_name + ".direction", glm::normalize(light->direction));
                    }
                    else if (light->lightType == lightType::SPOT)
                    {
                        command.material->shader->set(light_name + ".direction", glm::normalize(light->direction));
                        command.material->shader->set(light_name + ".inner_cone_angle", light->inner_cone_angle);
                        command.material->shader->set(light_name + ".outer_cone_angle", light->outer_cone_angle);
                    }
                    command.material->shader->set(light_name + ".position", light_position);
                    command.material->shader->set(light_name + ".color", light->color);
                    command.material->shader->set(light_name + ".attenuation", light->attenuation);
                    command.material->shader->set(light_name + ".type", (int)light->lightType);
                }
            }
            /////////////////////////// LIGHT COMPONENT ///////////////////////////

            command.mesh->draw();
        }

        // If there is a sky material, draw the sky
        if (this->skyMaterial)
        {
            // TODO: (Req 10) setup the sky material
            this->skyMaterial->setup();

            // TODO: (Req 10) Get the camera position
            glm::vec3 camPos = M * glm::vec4(0, 0, 0, 1);

            // TODO: (Req 10) Create a model matrix for the sky such that it always follows the camera (sky sphere center = camera position)
            glm::mat4 modelMatrix = glm::translate(glm::mat4(1.0f), camPos);

            // TODO: (Req 10) We want the sky to be drawn behind everything (in NDC space, z=1)
            //  We can achieve this by multiplying by an extra matrix after the projection but what values should we put in it?
            glm::mat4 alwaysBehindTransform = glm::mat4(
                1.0f, 0.0f, 0.0f, 0.0f,
                0.0f, 1.0f, 0.0f, 0.0f,
                0.0f, 0.0f, 0.0f, 0.0f,
                0.0f, 0.0f, 1.0f, 1.0f);

            // TODO: (Req 10) set the "transform" uniform
            skyMaterial->shader->set("transform", alwaysBehindTransform * VP * modelMatrix);

            // TODO: (Req 10) draw the sky sphere
            skySphere->draw();
        }

        // TODO: (Req 9) Draw all the transparent commands
        //  Don't forget to set the "transform" uniform to be equal the model-view-projection matrix for each render command
        for (const auto &command : transparentCommands)
        {
            command.material->setup();
            command.material->shader->set("transform", VP * command.localToWorld);
            /////////////////////////// ADD LIGHT COMPONENT HERE ///////////////////////////
            if (dynamic_cast<LitMaterial *>(command.material))
            {
                LitMaterial *litMaterial = dynamic_cast<LitMaterial *>(command.material);
                command.material->shader->set("camera_position", eye);
                command.material->shader->set("light_count", (int)lightCommands.size());
                command.material->shader->set("VP", VP);
                command.material->shader->set("M", command.localToWorld);
                command.material->shader->set("M_IT", glm::transpose(glm::inverse(command.localToWorld)));

                for (size_t i = 0; i < lightCommands.size(); i++)
                {
                    LightComponent *light = lightCommands[i];
                    glm::vec3 light_position;
                    std::string light_name = "lights[" + std::to_string(i) + "]";
                    if (light->getOwner()->parent)
                    {
                        light_position = light->getOwner()->parent->localTransform.position + light->getOwner()->localTransform.position;
                    }
                    else
                    {
                        light_position = light->getOwner()->localTransform.position;
                    }

                    if (light->lightType == lightType::DIRECTIONAL)
                    {
                        command.material->shader->set(light_name + ".direction", light->direction);
                    }
                    else if (light->lightType == lightType::SPOT)
                    {
                        command.material->shader->set(light_name + ".direction", light->direction);
                        command.material->shader->set(light_name + ".inner_cone_angle", light->inner_cone_angle);
                        command.material->shader->set(light_name + ".outer_cone_angle", light->outer_cone_angle);
                    }
                    command.material->shader->set(light_name + ".position", light_position);
                    command.material->shader->set(light_name + ".color", light->color);
                    command.material->shader->set(light_name + ".attenuation", light->attenuation);
                    command.material->shader->set(light_name + ".type", (int)light->lightType);
                }
            }
            /////////////////////////// LIGHT COMPONENT ///////////////////////////
            command.mesh->draw();
        }
        if (debug == true)
        {
            // Option 1: Show only wireframes (kart will appear white/gray, wheels blue)
            debugDrawer.setDebugMode(btIDebugDraw::DBG_DrawWireframe + btIDebugDraw::DBG_DrawConstraints);

            // Option 2: Show wireframes + contact points (current - kart appears red due to contacts)
            // debugDrawer.setDebugMode(btIDebugDraw::DBG_DrawWireframe + btIDebugDraw::DBG_DrawContactPoints + btIDebugDraw::DBG_DrawConstraints + btIDebugDraw::DBG_DrawConstraintLimits);

            // Option 3: Show everything for full debug info
            // debugDrawer.setDebugMode(btIDebugDraw::DBG_DrawWireframe + btIDebugDraw::DBG_DrawContactPoints + btIDebugDraw::DBG_DrawConstraints + btIDebugDraw::DBG_DrawConstraintLimits + btIDebugDraw::DBG_DrawAabb);

            dynWorld->debugDrawWorld();
            debugDrawer.glfw3_device_render(glm::value_ptr(VP));
        }
        // If there is a postprocess material, apply postprocessing
        if (postprocessMaterial)
        {
            // Bind default framebuffer for postprocessing output
            glBindFramebuffer(GL_FRAMEBUFFER, 0);

            // Clear any existing viewport settings
            glViewport(0, 0, windowSize.x, windowSize.y);

            postprocessMaterial->setup();
            glBindVertexArray(postProcessVertexArray);
            glDrawArrays(GL_TRIANGLES, 0, 3);
        }
    }

    void ForwardRenderer::loadFont(const std::string &fontPath)
    {
        // Load font face
        if (FT_New_Face(ft, fontPath.c_str(), 0, &face))
        {
            std::cerr << "ERROR::FREETYPE: Failed to load font: " << fontPath << std::endl;
            return;
        }

        // Set size to load glyphs as
        FT_Set_Pixel_Sizes(face, 0, 48);

        // Disable byte-alignment restriction
        glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

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
            // Now store character for later use
            Character character = {
                texture,
                glm::ivec2(face->glyph->bitmap.width, face->glyph->bitmap.rows),
                glm::ivec2(face->glyph->bitmap_left, face->glyph->bitmap_top),
                static_cast<GLuint>(face->glyph->advance.x)};
            characters.insert(std::pair<char, Character>(c, character));
        }
        glBindTexture(GL_TEXTURE_2D, 0);
    }

    void ForwardRenderer::initializeTextRendering()
    {
        // Initialize FreeType
        if (FT_Init_FreeType(&ft))
        {
            std::cerr << "ERROR::FREETYPE: Could not init FreeType Library" << std::endl;
            return;
        }

        // Load font
        loadFont("assets/fonts/arial.ttf");

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

        // Load text shader
        textShader = new ShaderProgram();
        textShader->attach("assets/shaders/text.vert", GL_VERTEX_SHADER);
        textShader->attach("assets/shaders/text.frag", GL_FRAGMENT_SHADER);
        textShader->link();
    }

    void ForwardRenderer::destroyTextRendering()
    {
        // Clean up character textures
        for (auto &pair : characters)
        {
            glDeleteTextures(1, &pair.second.textureID);
        }
        characters.clear();

        // Clean up FreeType
        FT_Done_Face(face);
        FT_Done_FreeType(ft);

        // Clean up OpenGL resources
        glDeleteVertexArrays(1, &textVAO);
        glDeleteBuffers(1, &textVBO);
        if (textShader)
        {
            delete textShader;
            textShader = nullptr;
        }
    }

    void ForwardRenderer::renderText(const std::string &text, float x, float y, float scale, const glm::vec3 &color)
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
        textShader->use();
        textShader->set("textColor", color);

        // Set up projection matrix for 2D rendering
        glm::mat4 projection = glm::ortho(0.0f, static_cast<float>(windowSize.x),
                                          0.0f, static_cast<float>(windowSize.y));
        textShader->set("projection", projection);

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

        glBindVertexArray(0);
        glBindTexture(GL_TEXTURE_2D, 0);

        // Restore previous OpenGL state
        if (!blendEnabled)
            glDisable(GL_BLEND);
        if (depthTestEnabled)
            glEnable(GL_DEPTH_TEST);
    }

    void ForwardRenderer::renderTextCentered(const std::string &text, float x, float y, float scale, const glm::vec3 &color)
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
}