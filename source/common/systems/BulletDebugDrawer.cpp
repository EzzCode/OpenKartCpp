#include "BulletDebugDrawer.hpp"

#include <algorithm>
#include <cstdint>
#include <iostream>
#include <glad/gl.h>
#include <cassert>

#define MAX_LINES_DRAWCALL 10000

GLuint dev_program;
GLint dev_uniform_proj;
GLint dev_uniform_col;
GLint dev_attrib_pos;

GLuint dev_vao;
GLuint dev_vbo;

BulletDebugDrawer::BulletDebugDrawer() : m_debugMode(0) {

}

void BulletDebugDrawer::drawLine(const btVector3& from,const btVector3& to, const btVector3& color) {
    m_lines.push_back(from.getX());
    m_lines.push_back(from.getY());
    m_lines.push_back(from.getZ());

    m_lines.push_back(to.getX());
    m_lines.push_back(to.getY());
    m_lines.push_back(to.getZ());
}

void BulletDebugDrawer::setDebugMode(int debugMode) {
    m_debugMode = debugMode;
}

int BulletDebugDrawer::getDebugMode() const {
    return m_debugMode;
}

void BulletDebugDrawer::reportErrorWarning(const char* warningString) {
    std::cout << warningString << std::endl;
}

void BulletDebugDrawer::glfw3_device_create(void) {
    GLint status;
    static const GLchar *vertex_shader =
        "#version 150\n"
        "uniform mat4 ProjMtx;\n"
        "in vec3 Position;\n"
        "void main() {\n"
        "   gl_Position = ProjMtx * vec4(Position, 1);\n"
        "}\n";
    static const GLchar *fragment_shader =
        "#version 150\n"
        "uniform vec3 Color;\n"
        "out vec4 Out_Color;\n"
        "void main(){\n"
        "   Out_Color = vec4(Color, 1);\n"
        "}\n";

    dev_program = glCreateProgram();
    GLuint vert_shdr = glCreateShader(GL_VERTEX_SHADER);
    GLuint frag_shdr = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(vert_shdr, 1, &vertex_shader, 0);
    glShaderSource(frag_shdr, 1, &fragment_shader, 0);
    glCompileShader(vert_shdr);
    glCompileShader(frag_shdr);
    glGetShaderiv(vert_shdr, GL_COMPILE_STATUS, &status);
    assert(status == GL_TRUE);
    glGetShaderiv(frag_shdr, GL_COMPILE_STATUS, &status);
    assert(status == GL_TRUE);
    glAttachShader(dev_program, vert_shdr);
    glAttachShader(dev_program, frag_shdr);
    glLinkProgram(dev_program);
    glGetProgramiv(dev_program, GL_LINK_STATUS, &status);
    assert(status == GL_TRUE);
    glDetachShader(dev_program, vert_shdr);
        glDetachShader(dev_program, frag_shdr);
        glDeleteShader(vert_shdr);
        glDeleteShader(frag_shdr);

    dev_uniform_proj = glGetUniformLocation(dev_program, "ProjMtx");
    dev_uniform_col = glGetUniformLocation(dev_program, "Color");
    dev_attrib_pos = glGetAttribLocation(dev_program, "Position");

    {
        /* buffer setup */
        glGenBuffers(1, &dev_vbo);
        glGenVertexArrays(1, &dev_vao);

        glBindVertexArray(dev_vao);
        glBindBuffer(GL_ARRAY_BUFFER, dev_vbo);
        glBufferData(GL_ARRAY_BUFFER, MAX_LINES_DRAWCALL * 24, nullptr, GL_STREAM_DRAW);

        glEnableVertexAttribArray(dev_attrib_pos);

        glVertexAttribPointer(dev_attrib_pos, 3, GL_FLOAT, GL_FALSE, 12, 0);
    }

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

void BulletDebugDrawer::glfw3_device_render(const float *matrix) {
    glUseProgram(dev_program);
    glUniformMatrix4fv(dev_uniform_proj, 1, GL_FALSE, matrix);
    glUniform3f(dev_uniform_col, 1.0f, 0.0f, 0.0f);

    glBindVertexArray(dev_vao);
    glBindBuffer(GL_ARRAY_BUFFER, dev_vbo);

    for (int i = 0; i < m_lines.size(); i += 2 * MAX_LINES_DRAWCALL) {
        int batchVertexCount = std::min<int>(m_lines.size() - i, 2 * MAX_LINES_DRAWCALL);
        glBufferSubData(GL_ARRAY_BUFFER, 0, batchVertexCount * 12, reinterpret_cast<void *>(m_lines.data() + i));
        glDrawArrays(GL_LINES, 0, batchVertexCount);
    }

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
    glUseProgram(0);

    m_lines.clear();
}

void BulletDebugDrawer::glfw3_device_destroy(void) {
    glDeleteProgram(dev_program);
    glDeleteBuffers(1, &dev_vbo);
    glDeleteVertexArrays(1, &dev_vao);
}