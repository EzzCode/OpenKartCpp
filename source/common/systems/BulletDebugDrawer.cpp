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
GLuint dev_color_vbo; // Make color VBO global to persist between frames

BulletDebugDrawer::BulletDebugDrawer() : m_debugMode(0), m_initialized(false) {

}

BulletDebugDrawer::~BulletDebugDrawer() {
    if (m_initialized) {
        glfw3_device_destroy();
    }
}

void BulletDebugDrawer::drawLine(const btVector3& from, const btVector3& to, const btVector3& color) {
    // Store line vertices
    m_lines.push_back(from.getX());
    m_lines.push_back(from.getY());
    m_lines.push_back(from.getZ());

    m_lines.push_back(to.getX());
    m_lines.push_back(to.getY());
    m_lines.push_back(to.getZ());
    
    // Store corresponding colors for each vertex
    m_colors.push_back(color.getX());
    m_colors.push_back(color.getY());
    m_colors.push_back(color.getZ());
    
    m_colors.push_back(color.getX());
    m_colors.push_back(color.getY());
    m_colors.push_back(color.getZ());
}

void BulletDebugDrawer::setDebugMode(int debugMode) {
    m_debugMode = debugMode;
}

int BulletDebugDrawer::getDebugMode() const {
    return m_debugMode;
}

void BulletDebugDrawer::reportErrorWarning(const char* warningString) {
    std::cerr << "[Bullet Physics Warning]: " << warningString << std::endl;
}

void BulletDebugDrawer::glfw3_device_create(void) {
    if (m_initialized) {
        return; // Already initialized
    }
    
    GLint status;
    static const GLchar *vertex_shader =
        "#version 150\n"
        "uniform mat4 ProjMtx;\n"
        "in vec3 Position;\n"
        "in vec3 Color;\n"
        "out vec3 FragColor;\n"
        "void main() {\n"
        "   gl_Position = ProjMtx * vec4(Position, 1);\n"
        "   FragColor = Color;\n"
        "}\n";
    static const GLchar *fragment_shader =
        "#version 150\n"
        "in vec3 FragColor;\n"
        "out vec4 Out_Color;\n"
        "void main(){\n"
        "   Out_Color = vec4(FragColor, 1);\n"
        "}\n";

    dev_program = glCreateProgram();
    GLuint vert_shdr = glCreateShader(GL_VERTEX_SHADER);
    GLuint frag_shdr = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(vert_shdr, 1, &vertex_shader, 0);
    glShaderSource(frag_shdr, 1, &fragment_shader, 0);
    glCompileShader(vert_shdr);    glCompileShader(frag_shdr);
    glGetShaderiv(vert_shdr, GL_COMPILE_STATUS, &status);
    if (status != GL_TRUE) {
        char log[512];
        glGetShaderInfoLog(vert_shdr, 512, nullptr, log);
        std::cerr << "Vertex shader compilation failed: " << log << std::endl;
    }
    glGetShaderiv(frag_shdr, GL_COMPILE_STATUS, &status);
    if (status != GL_TRUE) {
        char log[512];
        glGetShaderInfoLog(frag_shdr, 512, nullptr, log);
        std::cerr << "Fragment shader compilation failed: " << log << std::endl;
    }
    glAttachShader(dev_program, vert_shdr);
    glAttachShader(dev_program, frag_shdr);
    glLinkProgram(dev_program);
    glGetProgramiv(dev_program, GL_LINK_STATUS, &status);
    if (status != GL_TRUE) {
        char log[512];
        glGetProgramInfoLog(dev_program, 512, nullptr, log);
        std::cerr << "Shader program linking failed: " << log << std::endl;
    }    glDetachShader(dev_program, vert_shdr);
        glDetachShader(dev_program, frag_shdr);
        glDeleteShader(vert_shdr);
        glDeleteShader(frag_shdr);

    dev_uniform_proj = glGetUniformLocation(dev_program, "ProjMtx");
    dev_attrib_pos = glGetAttribLocation(dev_program, "Position");
    GLint dev_attrib_col = glGetAttribLocation(dev_program, "Color");    {
        /* buffer setup */
        glGenBuffers(1, &dev_vbo);
        glGenBuffers(1, &dev_color_vbo); // Create persistent color buffer
        glGenVertexArrays(1, &dev_vao);

        glBindVertexArray(dev_vao);
        
        // Position buffer
        glBindBuffer(GL_ARRAY_BUFFER, dev_vbo);
        glBufferData(GL_ARRAY_BUFFER, MAX_LINES_DRAWCALL * 24, nullptr, GL_STREAM_DRAW);
        glEnableVertexAttribArray(dev_attrib_pos);
        glVertexAttribPointer(dev_attrib_pos, 3, GL_FLOAT, GL_FALSE, 12, 0);
        
        // Color buffer - bind to correct attribute
        glBindBuffer(GL_ARRAY_BUFFER, dev_color_vbo);
        glBufferData(GL_ARRAY_BUFFER, MAX_LINES_DRAWCALL * 24, nullptr, GL_STREAM_DRAW);
        glEnableVertexAttribArray(dev_attrib_col);
        glVertexAttribPointer(dev_attrib_col, 3, GL_FLOAT, GL_FALSE, 12, 0);
    }

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
    
    m_initialized = true;
}

void BulletDebugDrawer::glfw3_device_render(const float *matrix) {
    if (!m_initialized || m_lines.empty()) {
        return;
    }

    glUseProgram(dev_program);
    glUniformMatrix4fv(dev_uniform_proj, 1, GL_FALSE, matrix);

    glBindVertexArray(dev_vao);

    // Upload vertex data in batches
    for (int i = 0; i < m_lines.size(); i += 2 * MAX_LINES_DRAWCALL * 3) {
        int batchVertexCount = std::min<int>((m_lines.size() - i) / 3, 2 * MAX_LINES_DRAWCALL);
        
        // Upload position data
        glBindBuffer(GL_ARRAY_BUFFER, dev_vbo);
        glBufferSubData(GL_ARRAY_BUFFER, 0, batchVertexCount * 12, 
                       reinterpret_cast<const void*>(m_lines.data() + i));
        
        // Upload color data to persistent color buffer
        glBindBuffer(GL_ARRAY_BUFFER, dev_color_vbo);
        glBufferSubData(GL_ARRAY_BUFFER, 0, batchVertexCount * 12, 
                       reinterpret_cast<const void*>(m_colors.data() + i));
        
        glDrawArrays(GL_LINES, 0, batchVertexCount);
    }

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
    glUseProgram(0);

    // Clear the line data after rendering
    clearLines();
}

void BulletDebugDrawer::glfw3_device_destroy(void) {
    if (!m_initialized) {
        return;
    }
    
    glDeleteProgram(dev_program);
    glDeleteBuffers(1, &dev_vbo);
    glDeleteBuffers(1, &dev_color_vbo); // Clean up color buffer
    glDeleteVertexArrays(1, &dev_vao);
    
    m_initialized = false;
}