#include "ColorMesh.h"

shadow::ColorMesh::ColorMesh(const std::vector<Vertex>& vertices, const std::vector<GLuint>& indices, glm::vec3 color)
: color(color), indexCount(static_cast<GLsizei>(indices.size()))
{
    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);
    glGenBuffers(1, &ebo);

    glBindVertexArray(vao);

    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), &vertices[0], GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(GLuint), &indices[0], GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), static_cast<void*>(nullptr));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), reinterpret_cast<void*>(offsetof(Vertex, normal)));

    glBindVertexArray(0);
}

void shadow::ColorMesh::setColor(glm::vec3 color)
{
    this->color = color;
}

glm::vec3 shadow::ColorMesh::getColor() const
{
    return color;
}

void shadow::ColorMesh::draw(std::shared_ptr<GLShader> shader) const
{
    shader->use();
    glBindVertexArray(vao);
    glDrawElements(GL_TRIANGLES, indexCount, GL_UNSIGNED_INT, nullptr);
    glBindVertexArray(0);
}
