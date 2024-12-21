#include "forward-renderer.hpp"
#include "../mesh/mesh-utils.hpp"
#include "../texture/texture-utils.hpp"

namespace our
{

    void ForwardRenderer::initialize(glm::ivec2 windowSize, const nlohmann::json &config)
    {
        // First, we store the window size for later use
        this->windowSize = windowSize;

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

            // generate frame buffer
            glGenFramebuffers(1, &postprocessFrameBuffer);

            // bind the postprocess frame buffer
            glBindFramebuffer(GL_FRAMEBUFFER, postprocessFrameBuffer);

            // TODO: (Req 11) Create a color and a depth texture and attach them to the framebuffer
            //  Hints: The color format can be (Red, Green, Blue and Alpha components with 8 bits for each channel).
            //  The depth format can be (Depth component with 24 bits).

            // create color texture with GL_RGBA8 internal formal
            colorTarget = texture_utils::empty(GL_RGBA8, windowSize);

            // create depth texture with GL_DEPTH_COMPONENT24 internal format
            depthTarget = texture_utils::empty(GL_DEPTH_COMPONENT24, windowSize);

            // void glFramebufferTexture1D(
            //       GLenum target, => target framebuffer (GL_FRAMEBUFFER, GL_DRAW_FRAMEBUFFER, ..),
            //       GLenum attachment =>  point of attachment,
            //       GLenum textarget => type of texture expected in the texture paramenter,
            //       GLuint texture => texture object to be attached,
            //       GLint level => level of detail
            //  )

            // attach color texture to the framebuffer
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, colorTarget->getOpenGLName(), 0);

            // attach depth texture to the framebuffer
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthTarget->getOpenGLName(), 0);

            // TODO: (Req 11) Unbind the framebuffer just to be safe

            // write to default framebuffer
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
    }

    void ForwardRenderer::destroy()
    {
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
    }

    void ForwardRenderer::render(World *world)
    {
        world->deleteMarkedEntities();
        // First of all, we search for a camera and for all the mesh renderers
        CameraComponent *camera = nullptr;
        opaqueCommands.clear();
        transparentCommands.clear();
        // !!!!!!!!!!!!!!!! SHOULD WE CLEAR THE LIGHT ????????
        lights.clear();
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

            // !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! get all light sources
            if (auto light = entity->getComponent<LightComponent>(); light)
            {
                lights.push_back(light);
            }
        }

        // If there is no camera, we return (we cannot render without a camera)
        if (camera == nullptr)
            return;

        // TODO: (Req 9) Modify the following line such that "cameraForward" contains a vector pointing the camera forward direction
        //  HINT: See how you wrote the CameraComponent::getViewMatrix, it should help you solve this one
        // glm::vec3 cameraForward = glm::vec3(0.0, 0.0, -1.0f);
        
        auto M = camera->getOwner()->getLocalToWorldMatrix();
        glm::vec3 eye = M * glm::vec4(0, 0, 0, 1);
        glm::vec3 center = M * glm::vec4(0, 0, -1, 1);
        // the vector containing the camera direction
        glm::vec3 cameraForward = camera->getOwner()->getLocalToWorldMatrix() * glm::vec4(0, 0, -1, 0);

        std::sort(transparentCommands.begin(), transparentCommands.end(), [cameraForward](const RenderCommand &first, const RenderCommand &second)
                  {
            //TODO: (Req 9) Finish this function
            // HINT: the following return should return true "first" should be drawn before "second"
            
            // farthest (bigger value) to nearest (smaller value)
            if (glm::dot(first.center, cameraForward) > glm::dot(second.center, cameraForward))
                return true; 
            return false; });

        // TODO: (Req 9) Get the camera ViewProjection matrix and store it in VP
        glm::mat4 VP = camera->getProjectionMatrix(windowSize) * camera->getViewMatrix();

        // TODO: (Req 9) Set the OpenGL viewport using viewportStart and viewportSize

        // glviewport(x,y, windowSize.x, windowSize.y)
        //      x and y specifies the lower left corner of the viewport rectangle, in pixels. The initial value is (0,0).
        //      windowSize specifies the viewport siez
        glViewport(0, 0, windowSize.x, windowSize.y);

        // TODO: (Req 9) Set the clear color to black and the clear depth to 1
        glClearColor(0.0f, 0.0f, 0.0f, 0.0f);

        // specify the value used when the depth is cleared
        glClearDepth(1);

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

        // create commands vector to contain opaque commands
        std::vector<RenderCommand> coms(opaqueCommands);

        for (RenderCommand &com : coms)
        {

            // setup pipline and set the program to be used
            com.material->setup();

            // send to uniform the matrix to transform position
            com.material->shader->set("transform", VP * com.localToWorld);
            // !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
            if (dynamic_cast<LitMaterial *>(com.material))
            {
                LitMaterial *litMaterial = dynamic_cast<LitMaterial *>(com.material);

                // set VP, eye, M, M_IT and light count in shader
                com.material->shader->set("VP", VP);
                com.material->shader->set("eye", eye);
                com.material->shader->set("M", com.localToWorld);
                com.material->shader->set("M_IT", glm::inverse(com.localToWorld));
                com.material->shader->set("light_count", (int)lights.size());

                for (int i = 0; i < lights.size(); i++)
                {

                    glm::vec3 lightPos;
                    std::string light_name = "lights[" + std::to_string(i) + "]";
                    // if it is a child, send parent pos + relative pos of entity
                    if (lights[i]->getOwner()->parent)
                    {
                        // If light has a parent, add parent position to light's relative position
                        lightPos = lights[i]->getOwner()->parent->localTransform.position + lights[i]->getOwner()->localTransform.position;
                    }
                    else
                    {
                        // If light has no parent, use its own position
                        lightPos = lights[i]->getOwner()->localTransform.position;
                    }

                    // for directional and Spot, send read directions
                    //    Set light type-specific variables in shader

                    /* 
                        Directional and spot lights have a direction associated with them, which is used to calculate the light's intensity at a given surface. These directions are specified in world space, and are used as-is in the lighting calculations.

                        Point lights, on the other hand, do not have a direction associated with them. Instead, they have a position in world space and their intensity falls off with distance from the surface being shaded. In this case, the distance from the surface to the light is used in the lighting calculations, rather than the direction.
                    
                    */
                    switch (lights[i]->lightType)
                    {
                     // Normalize direction vector for directional lights
                    case LightType::DIRECTIONAL:
                        com.material->shader->set(light_name + ".direction", glm::normalize(lights[i]->direction));
                        break;
                    case LightType::POINT:
                        break;
                    case LightType::SPOT:
                        // Normalize direction vector for spot lights
                        com.material->shader->set(light_name + ".direction", glm::normalize(lights[i]->direction));
                        com.material->shader->set(light_name + ".cone_angles", lights[i]->cone_angles);
                        break;
                    }
                    // Construct light variable name based on light index
                    com.material->shader->set(light_name + ".position", lightPos);
                    // Set remaining light variables in shader
                    com.material->shader->set(light_name + ".color", lights[i]->color);
                    com.material->shader->set(light_name + ".attenuation", lights[i]->attenuation);
                    com.material->shader->set(light_name + ".type", (int)lights[i]->lightType);
                }
            }
            // !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

            // draw the mesh
            com.mesh->draw();
        }

        // If there is a sky material, draw the sky
        if (this->skyMaterial)
        {
            // TODO: (Req 10) setup the sky material
            this->skyMaterial->setup();
            // TODO: (Req 10) Get the camera position
            glm::vec3 camPos = camera->getOwner()->getLocalToWorldMatrix() * glm::vec4(0, 0, 0, 1);

            // TODO: (Req 10) Create a model matrix for the sy such that it always follows the camera (sky sphere center = camera position)
            glm::mat4 matrix = glm::translate(glm::mat4(1.0f), camPos);

            // TODO: (Req 10) We want the sky to be drawn behind everything (in NDC space, z=1)
            //  We can acheive the is by multiplying by an extra matrix after the projection but what values should we put in it?

            // make z = w || COLUMN MAJOR MATRIX ORDERING || Transpose
            glm::mat4 alwaysBehindTransform = glm::mat4(
                1.0f, 0.0f, 0.0f, 0.0f,
                0.0f, 1.0f, 0.0f, 0.0f,
                0.0f, 0.0f, 0.0f, 0.0f,
                0.0f, 0.0f, 1.0f, 1.0f);
            // TODO: (Req 10) set the "transform" uniform
            skyMaterial->shader->set("transform", alwaysBehindTransform * VP * matrix);
            // TODO: (Req 10) draw the sky sphere
            skySphere->draw();
        }
        // TODO: (Req 9) Draw all the transparent commands
        //  Don't forget to set the "transform" uniform to be equal the model-view-projection matrix for each render command

        std::vector<RenderCommand> coms2(transparentCommands);

        // append to it the transparnt commands
        // coms.insert(coms.end(), transparentCommands.begin(), transparentCommands.end());

        for (RenderCommand &com : coms2)
        {
            // ShaderProgram* shader = com.material->shader;
            // Mash* mesh = com.mesh;

            com.material->setup();

            com.material->shader->set("transform", VP * com.localToWorld);
            // com.material->shader->set("tint", VP * com.localToWorld);

            // !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
            if (dynamic_cast<LitMaterial *>(com.material))
            {
                LitMaterial *litMaterial = dynamic_cast<LitMaterial *>(com.material);

                // set VP, eye, M, M_IT and light count in shader
                com.material->shader->set("VP", VP);
                com.material->shader->set("eye", eye);
                com.material->shader->set("M", com.localToWorld);
                com.material->shader->set("M_IT", glm::inverse(com.localToWorld));
                com.material->shader->set("light_count", (int)lights.size());

                for (int i = 0; i < lights.size(); i++)
                {

                    glm::vec3 lightPos;
                    std::string light_name = "lights[" + std::to_string(i) + "]";
                    // if it is a child, send parent pos + relative pos of entity
                    if (lights[i]->getOwner()->parent)
                    {
                        lightPos = lights[i]->getOwner()->parent->localTransform.position + lights[i]->getOwner()->localTransform.position;
                    }
                    else
                    {
                        lightPos = lights[i]->getOwner()->localTransform.position;
                    }

                    // for directional/Spot  send read directions
                    switch (lights[i]->lightType)
                    {
                    case LightType::DIRECTIONAL:
                        com.material->shader->set(light_name + ".direction", glm::normalize(lights[i]->direction));
                        break;
                    case LightType::POINT:
                        break;
                    case LightType::SPOT:
                        com.material->shader->set(light_name + ".direction", glm::normalize(lights[i]->direction));
                        com.material->shader->set(light_name + ".cone_angles", lights[i]->cone_angles);
                        break;
                    }
                    com.material->shader->set(light_name + ".position", lightPos);
                    com.material->shader->set(light_name + ".color", lights[i]->color);
                    com.material->shader->set(light_name + ".attenuation", lights[i]->attenuation);
                    com.material->shader->set(light_name + ".type", (int)lights[i]->lightType);
                }
            }
            // !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

            com.mesh->draw();
        }

        // If there is a postprocess material, apply postprocessing
        if (postprocessMaterial)
        {
            // TODO: (Req 11) Return to the default framebuffer
            glBindFramebuffer(GL_FRAMEBUFFER, 0);

            // TODO: (Req 11) Setup the postprocess material and draw the fullscreen triangle

            postprocessMaterial->setup();

            // use another texture unit for drawing
            glActiveTexture(GL_TEXTURE1);

            // bind depth texture
            depthTarget->bind();

            // bind the sampler to TEXTURE UNIT 1
            postprocessMaterial->sampler->bind(1);

            // !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
            // set depth_sampler to 1
            postprocessMaterial->shader->set("depth_sampler", 1);

            // set inverse projection matrix
            postprocessMaterial->shader->set("inverse_projection", glm::inverse(camera->getProjectionMatrix(windowSize)));
            // !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

            // select GL_TEXTURE0 as active txture unit
            glActiveTexture(GL_TEXTURE0);

            // bind vertix array
            glBindVertexArray(postProcessVertexArray);

            // draw triangle
            //      primitive_mode, start, count
            glDrawArrays(GL_TRIANGLES, 0, 3);
        }
    }

}