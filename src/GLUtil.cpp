#include "GLUtil.h"

#include <vec3.hpp>
#include <vector>
#include <GL/glew.h>


//TODO: seems to not unbind resources after construction, valve pls fix, also seems to just throw away the handles to the buffers, could cause leaks
unsigned GLUtil::BuildVAOfromData(const std::vector<float>& vertexes, 
                                const std::vector<float>& colours, 
                                const std::vector<unsigned>& indexes)
{
    //ids for buffer objects
    unsigned vbo[2], vao, ibo;

    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    //generate vertex buffers and assign vertexes to first
    glGenBuffers(2, vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo[0]);
    glBufferData(GL_ARRAY_BUFFER, vertexes.size() * sizeof(float), vertexes.data(), GL_STATIC_DRAW);

    //shader layout location for vertexes
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), 0);
    glEnableVertexAttribArray(0);

    //assign colours to second
    glBindBuffer(GL_ARRAY_BUFFER, vbo[1]);
    glBufferData(GL_ARRAY_BUFFER, colours.size() * sizeof(float), colours.data(), GL_STATIC_DRAW);

    //shader layout location for colours
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), 0);
    glEnableVertexAttribArray(1);

    //generate index buffer and assign indexes to it
    glGenBuffers(1, &ibo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indexes.size() * sizeof(unsigned), indexes.data(), GL_STATIC_DRAW);

    glBindVertexArray(0);
    return vao;
}

unsigned GLUtil::BuildVAOfromData(const std::vector<glm::vec3>& vertexes, 
                                const std::vector<glm::vec3>& colours, 
                                const std::vector<unsigned>& indexes)
{
    //ids for buffer objects
    unsigned vbo[2], vao, ibo;

    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    //generate vertex buffers and assign vertexes to first
    glGenBuffers(2, vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo[0]);
    glBufferData(GL_ARRAY_BUFFER, vertexes.size() * sizeof(glm::vec3), vertexes.data(), GL_STATIC_DRAW);

    //shader layout location for vertexes
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), 0);
    glEnableVertexAttribArray(0);

    //assign colours to second
    glBindBuffer(GL_ARRAY_BUFFER, vbo[1]);
    glBufferData(GL_ARRAY_BUFFER, colours.size() * sizeof(glm::vec3), colours.data(), GL_STATIC_DRAW);

    //shader layout location for colours
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), 0);
    glEnableVertexAttribArray(1);

    //generate index buffer and assign indexes to it
    glGenBuffers(1, &ibo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indexes.size() * sizeof(unsigned), indexes.data(), GL_STATIC_DRAW);

    glBindVertexArray(0);
    return vao;
}

unsigned GLUtil::BuildVAOfromData(const std::vector<glm::vec3>& vertexes, 
                                const std::vector<glm::vec3>& colours, 
                                const std::vector<unsigned>& indexes,
								const std::vector<glm::vec3>& normals)
{
    //ids for buffer objects
    unsigned vbo[3], vao, ibo;

    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    //generate vertex buffers and assign vertexes to first
    glGenBuffers(3, vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo[0]);
    glBufferData(GL_ARRAY_BUFFER, vertexes.size() * sizeof(glm::vec3), vertexes.data(), GL_STATIC_DRAW);

    //shader layout location for vertexes
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), 0);
    glEnableVertexAttribArray(0);

    //assign colours to second
    glBindBuffer(GL_ARRAY_BUFFER, vbo[1]);
    glBufferData(GL_ARRAY_BUFFER, colours.size() * sizeof(glm::vec3), colours.data(), GL_STATIC_DRAW);

    //shader layout location for colours
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), 0);
    glEnableVertexAttribArray(1);

    //assign normals to third
    glBindBuffer(GL_ARRAY_BUFFER, vbo[1]);
    glBufferData(GL_ARRAY_BUFFER, colours.size() * sizeof(glm::vec3), colours.data(), GL_STATIC_DRAW);

    //shader layout location for normals    
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), 0);
    glEnableVertexAttribArray(2);

    //generate index buffer and assign indexes to it
    glGenBuffers(1, &ibo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indexes.size() * sizeof(unsigned), indexes.data(), GL_STATIC_DRAW);

    glBindVertexArray(0);
    return vao;
}

unsigned GLUtil::BuildVAOfromData(const std::vector<glm::vec3>& vertexes, 
								const std::vector<glm::vec2>& texCoords)
{
    //ids for buffer objects
    unsigned vbo[2], vao;

    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    //generate vertex buffers and assign vertexes to first
    glGenBuffers(2, vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo[0]);
    glBufferData(GL_ARRAY_BUFFER, vertexes.size() * sizeof(glm::vec3), vertexes.data(), GL_STATIC_DRAW);

    //shader layout location for vertexes
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), 0);
    glEnableVertexAttribArray(0);

    //assign colours to second
    glBindBuffer(GL_ARRAY_BUFFER, vbo[1]);
    glBufferData(GL_ARRAY_BUFFER, texCoords.size() * sizeof(glm::vec2), texCoords.data(), GL_STATIC_DRAW);

    //shader layout location for colours
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), 0);
    glEnableVertexAttribArray(1);

    glBindVertexArray(0);
    return vao;
}