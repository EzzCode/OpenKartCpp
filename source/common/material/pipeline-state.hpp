#pragma once

#include <glad/gl.h>
#include <glm/vec4.hpp>
#include <json/json.hpp>

namespace our {
    // There are some options in the render pipeline that we cannot control via shaders
    // such as blending, depth testing and so on
    // Since each material could require different options (e.g. transparent materials usually use blending),
    // we will encapsulate all these options into a single structure that will also be responsible for configuring OpenGL's pipeline
    struct PipelineState {
        // This set of pipeline options specifies whether face culling will be used or not and how it will be configured
        struct {
            bool enabled = false;
            GLenum culledFace = GL_BACK;
            GLenum frontFace = GL_CCW;
        } faceCulling;

        // This set of pipeline options specifies whether depth testing will be used or not and how it will be configured
        struct {
            bool enabled = false;
            GLenum function = GL_LEQUAL;
        } depthTesting;

        // This set of pipeline options specifies whether blending will be used or not and how it will be configured
        struct {
            bool enabled = false;
            GLenum equation = GL_FUNC_ADD;
            GLenum sourceFactor = GL_SRC_ALPHA;
            GLenum destinationFactor = GL_ONE_MINUS_SRC_ALPHA;
            glm::vec4 constantColor = {0, 0, 0, 0};
        } blending;

        // These options specify the color and depth mask which can be used to
        // prevent the rendering/clearing from modifying certain channels of certain targets in the framebuffer
        glm::bvec4 colorMask = {true, true, true, true}; // To know how to use it, check glColorMask
        bool depthMask = true; // To know how to use it, check glDepthMask


        // This function should set the OpenGL options to the values specified by this structure
        // For example, if faceCulling.enabled is true, you should call glEnable(GL_CULL_FACE), otherwise, you should call glDisable(GL_CULL_FACE)
        void setup() const {
            //TODO: (Req 4) Write this function

            // check if face culling feature should be enabled
            if (faceCulling.enabled) {
                // enable face culling
                glEnable(GL_CULL_FACE);

                // specify whether front face, back face, or both are what will be candidates to be culled
                glCullFace(faceCulling.culledFace);

                // specifiy whether the front face is CW or CCW
                glFrontFace(faceCulling.frontFace);
            } else {
                glDisable(GL_CULL_FACE);
            }

            // check if depth testing should be enabled
            if (depthTesting.enabled) {
                // enable depth testing
                glEnable(GL_DEPTH_TEST);

                // specify the function used to compare between pixels depth value
                glDepthFunc(depthTesting.function);

            } else {
                glDisable(GL_DEPTH_TEST);
            }
        
            // check if blending should be enabled
            if(blending.enabled) {

                // enable blending
                glEnable(GL_BLEND);

                // specify the equation (add, subtract, reverse subt, min, max)
                //      SourceFactor*Source (Equation) DestinationFactor(Destination)
                //      GL_FUNC_ADD: add both terms
                glBlendEquation(blending.equation);
                
                // specify the source factor and the destination factor
                //    GL_SRC_ALPHA:           multiplies the source with sourchAlhpa
                //    GL_ONE_MINUS_SRC_ALPHA: multiplies the destination with 1-sourceAlpha 
                glBlendFunc(blending.sourceFactor, blending.destinationFactor);

                // specify the components of GL_BLEND_COLOR which is 
                // the constant color used for some of the options in source and 
                // distination factors like:
                //     GL_CONSTANT_COLOR, GL_ONE_MINUS_CONSTANT_COLOR
                glBlendColor(blending.constantColor.r, blending.constantColor.g, blending.constantColor.b, blending.constantColor.a);
            } else {
                glDisable(GL_BLEND);

               
            }
            
            // specifies if the depth buffer is enabled for writing or not
            glDepthMask(depthMask);
            
            // specify whether the individual color components in the frame buffer can or cannot be written for all draw buffers
            glColorMask(colorMask[0], colorMask[1], colorMask[2], colorMask[3]);

        }

        // Given a json object, this function deserializes a PipelineState structure
        void deserialize(const nlohmann::json& data);
    };

}