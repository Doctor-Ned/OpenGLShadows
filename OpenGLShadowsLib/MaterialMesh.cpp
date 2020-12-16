#include "MaterialMesh.h"
#include "ResourceManager.h"

shadow::MaterialMesh::MaterialMesh(const std::vector<Vertex>& vertices, const std::vector<GLuint>& indices, std::shared_ptr<Material> material)
    : material(material), uboMaterial(ResourceManager::getInstance().getUboMaterial()), indexCount(static_cast<GLsizei>(indices.size()))
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
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), reinterpret_cast<void*>(offsetof(Vertex, position)));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), reinterpret_cast<void*>(offsetof(Vertex, normal)));

    glBindVertexArray(0);
}

shadow::MaterialMesh::~MaterialMesh()
{
    glDeleteBuffers(1, &ebo);
    glDeleteBuffers(1, &vbo);
    glDeleteVertexArrays(1, &vao);
}

std::shared_ptr<shadow::MaterialMesh> shadow::MaterialMesh::fromPrimitiveData(std::shared_ptr<PrimitiveData> data, std::shared_ptr<Material> material)
{
    if (!data->isValid())
    {
        return {};
    }
    return std::make_shared<MaterialMesh>(data->toVertex(), data->getIndices(), material);
}

void shadow::MaterialMesh::setMaterial(std::shared_ptr<Material> material)
{
    this->material = material;
}

std::shared_ptr<shadow::Material> shadow::MaterialMesh::getMaterial() const
{
    return material;
}

void shadow::MaterialMesh::draw(std::shared_ptr<GLShader> shader) const
{
    draw(shader, true);
}

void shadow::MaterialMesh::draw(std::shared_ptr<GLShader> shader, bool updateMaterial) const
{
    if (updateMaterial)
    {
        uboMaterial->set(*material);
    }
    glBindVertexArray(vao);
    glDrawElements(GL_TRIANGLES, indexCount, GL_UNSIGNED_INT, nullptr);
    glBindVertexArray(0);
}

shadow::ShaderType shadow::MaterialMesh::getShaderType() const
{
    return ShaderType::Material;
}
