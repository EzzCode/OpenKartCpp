#ifndef SHADER_HPP
#define SHADER_HPP

#include <string>

#include <glad/gl.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glad/gl.h> // for GL_INVALID_INDEX

namespace our {

    class ShaderProgram {

    private:
        //Shader Program Handle (OpenGL object name)
        GLuint program;

    public:
        ShaderProgram(){
            //TODO: (Req 1) Create A shader program
            program = glCreateProgram();
        }
        ~ShaderProgram(){
            //TODO: (Req 1) Delete a shader program
            glDeleteProgram(program);
        }

        bool attach(const std::string &filename, GLenum type) const;

        bool link() const;

        void use() { 
            glUseProgram(program);
        }

        GLuint getUniformLocation(const std::string &name) {
            //TODO: (Req 1) Return the location of the uniform with the given name
            return glGetUniformLocation(program, name.c_str());
        }

        void set(const std::string &uniform, GLfloat value) {
            //TODO: (Req 1) Send the given float value to the given uniform
            GLuint location = getUniformLocation(uniform);
            if (location != GL_INVALID_INDEX)
                glUniform1f(location, value);
        }

        void set(const std::string &uniform, GLuint value) {
            //TODO: (Req 1) Send the given unsigned integer value to the given uniform
            GLuint location = getUniformLocation(uniform);
            if (location != GL_INVALID_INDEX)
                glUniform1ui(location, value);
        }

        void set(const std::string &uniform, GLint value) {
            //TODO: (Req 1) Send the given integer value to the given uniform
            GLuint location = getUniformLocation(uniform);
            if (location != GL_INVALID_INDEX)
                glUniform1i(location, value);
        }

        void set(const std::string &uniform, glm::vec2 value) {
            //TODO: (Req 1) Send the given 2D vector value to the given uniform
            GLuint location = getUniformLocation(uniform);
            if (location != GL_INVALID_INDEX)
                glUniform2f(location, value.x,value.y);
        }

        void set(const std::string &uniform, glm::vec3 value) {
            //TODO: (Req 1) Send the given 3D vector value to the given uniform
            GLuint location = getUniformLocation(uniform);
            if (location != GL_INVALID_INDEX)
                glUniform3f(location, value.x,value.y,value.z);
        }

        void set(const std::string &uniform, glm::vec4 value) {
            //TODO: (Req 1) Send the given 4D vector value to the given uniform
            GLuint location = getUniformLocation(uniform);
            if (location != GL_INVALID_INDEX)
                glUniform4f(location, value.x, value.y, value.z,value.w);
        }

        void set(const std::string &uniform, glm::mat4 matrix) {
            //TODO: (Req 1) Send the given matrix 4x4 value to the given uniform
            GLuint location = getUniformLocation(uniform);
            if (location != GL_INVALID_INDEX)
                glUniformMatrix4fv(location, 1, GL_FALSE, glm::value_ptr(matrix));
        }


        //TODO: (Req 1) Delete the copy constructor and assignment operator.
        //Question: Why do we delete the copy constructor and assignment operator?
        ShaderProgram(const ShaderProgram&) = delete;
        ShaderProgram& operator=(const ShaderProgram&) = delete;
        //We need to delete the copy constructor because opengl objects like shader programs
        //are managed through IDS which are unique to the instance, if we copied a ShaderProgram
        //object, this can lead to problems like double deletion.
    };

}

#endif