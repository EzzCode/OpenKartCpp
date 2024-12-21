#pragma once

#include <glad/gl.h>
#include "vertex.hpp"

namespace our {

    #define ATTRIB_LOC_POSITION 0
    #define ATTRIB_LOC_COLOR    1
    #define ATTRIB_LOC_TEXCOORD 2
    #define ATTRIB_LOC_NORMAL   3

    class Mesh {
        // Here, we store the object names of the 3 main components of a mesh:
        // A vertex array object, A vertex buffer and an element buffer
        unsigned int VBO, EBO;
        unsigned int VAO;
        // We need to remember the number of elements that will be draw by glDrawElements 
        GLsizei elementCount;
    public:

        // The constructor takes two vectors:
        // - vertices which contain the vertex data.
        // - elements which contain the indices of the vertices out of which each rectangle will be constructed.
        // The mesh class does not keep a these data on the RAM. Instead, it should create
        // a vertex buffer to store the vertex data on the VRAM,
        // an element buffer to store the element data on the VRAM,
        // a vertex array object to define how to read the vertex & element buffer during rendering 
        Mesh(const std::vector<Vertex>& vertices, const std::vector<unsigned int>& elements)
        {
            //TODO: (Req 2) Write this function
            // remember to store the number of elements in "elementCount" since you will need it for drawing
            // For the attribute locations, use the constants defined above: ATTRIB_LOC_POSITION, ATTRIB_LOC_COLOR, etc

            // set the elementCount with the elements vector size
            elementCount = (GLsizei)elements.size();

            // Generate & bind the vertex array to store the buffer access details
            glGenVertexArrays(1, &VAO);
            glBindVertexArray(VAO);


            //create and bind vertex buffer
            glGenBuffers(1, &VBO);
            glBindBuffer(GL_ARRAY_BUFFER, VBO);

            // put inside the buffer the vertices's data
            //      the second parameter is the size of the buffer
            //      the third parameter is the pointer to the buffer data
            //      Use GL_STATIC_DRAW for date that will not change or will change rarely
            glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), &vertices[0], GL_STATIC_DRAW);


            //create and bind elements buffer
            glGenBuffers(1, &EBO);
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);

            // put inside the buffer the elements's data
            //      the second parameter is the size of the buffer
            //      the third parameter is the pointer to the buffer data
            //      Use GL_STATIC_DRAW for date that will not change or will change rarely
            glBufferData(GL_ELEMENT_ARRAY_BUFFER, elements.size() * sizeof(unsigned int), &elements[0], GL_STATIC_DRAW);

            //the attributes are
            //  vec3 position   (GL_FLOAT)
            //  vec3 color      (glm: uint)
            //  vec2 tex_cood   (GL_FLOAT)
            //  vec3 normal     (GL_FLOAT)

            // set and enable POSITION attribute
            //     the second attribute 3 because it is vec3
            //     the third attribute is GL_FLOAT because of the datatype
            //     the fourth attribute is GL_FALSE because we don't want normalization
            //     the fifth attribute is the stripe size of the vertex side
            //     the sixth attribute is the offset to the position attribute inside the vertex
            glVertexAttribPointer(ATTRIB_LOC_POSITION, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, position));
            glEnableVertexAttribArray(ATTRIB_LOC_POSITION);

            // set and enable COLOR attribute
            //    the fourth attribute here is GL_TRUE because we want to use normalized colors
            glVertexAttribPointer(ATTRIB_LOC_COLOR, 4, GL_UNSIGNED_BYTE, GL_TRUE, sizeof(Vertex), (void*)offsetof(Vertex, color));
            glEnableVertexAttribArray(ATTRIB_LOC_COLOR);

            // set and enable TEX_COORD attribute
            glVertexAttribPointer(ATTRIB_LOC_TEXCOORD, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, tex_coord));
            glEnableVertexAttribArray(ATTRIB_LOC_TEXCOORD);

            // set and enable NORMAL attribute
            glVertexAttribPointer(ATTRIB_LOC_NORMAL, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, normal));
            glEnableVertexAttribArray(ATTRIB_LOC_NORMAL);

            // unbind the vertex array used
            glBindVertexArray(0);

        }

        // this function should render the mesh
        void draw() 
        {
            //TODO: (Req 2) Write this function

            // First we bind the vertex array since it know how to send the data from the buffers to shader attributes
            glBindVertexArray(VAO);

            // then we draw
            //     the first parameter is the primitive type to be drawn
            //     the second parameter is the number of elements to be drawn
            //     the third parameter is the datatype
            //     the fourth parameter is the offset of the first index in the array
            glDrawElements(GL_TRIANGLES, elementCount, GL_UNSIGNED_INT, 0); 

            // unbind the vertex array used
            glBindVertexArray(0);
        }

        // this function should delete the vertex & element buffers and the vertex array object
        ~Mesh(){
            //TODO: (Req 2) Write this function

            // delete the vertix array, the vertices buffer, and the elements buffer
            glDeleteVertexArrays(1, &VAO);
            glDeleteBuffers(1, &VBO);
            glDeleteBuffers(1, &EBO);
        }

        Mesh(Mesh const &) = delete;
        Mesh &operator=(Mesh const &) = delete;
    };

}