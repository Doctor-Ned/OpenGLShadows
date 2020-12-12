#include "TextureMesh.h"

#include <utility>
#include "ShadowLog.h"

shadow::TextureMesh::TextureMesh(const std::vector<TextureVertex>& vertices, const std::vector<GLuint>& indices,
                                 std::map<TextureType, std::shared_ptr<Texture>> textures)
    : textures(std::move(textures)), indexCount(static_cast<GLsizei>(indices.size()))
{
    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);
    glGenBuffers(1, &ebo);

    glBindVertexArray(vao);

    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(TextureVertex), &vertices[0], GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(GLuint), &indices[0], GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(TextureVertex), static_cast<void*>(nullptr));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(TextureVertex), reinterpret_cast<void*>(offsetof(TextureVertex, normal)));
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(TextureVertex), reinterpret_cast<void*>(offsetof(TextureVertex, texCoords)));

    glBindVertexArray(0);
}

std::shared_ptr<shadow::TextureMesh> shadow::TextureMesh::fromPrimitiveData(std::shared_ptr<PrimitiveData> data, std::map<TextureType, std::shared_ptr<Texture>> textures)
{
    if (!data->isValid())
    {
        return {};
    }
    return std::make_shared<TextureMesh>(data->toTextureVertex(), data->getIndices(), textures);
}

void shadow::TextureMesh::draw(std::shared_ptr<GLShader> shader) const
{
    for (const std::pair<TextureType, std::shared_ptr<Texture>> texture : textures)
    {
        glActiveTexture(GL_TEXTURE0 + static_cast<GLuint>(texture.first));
        glBindTexture(GL_TEXTURE_2D, texture.second->getId());
    }
    glActiveTexture(GL_TEXTURE0);
    glBindVertexArray(vao);
    glDrawElements(GL_TRIANGLES, indexCount, GL_UNSIGNED_INT, nullptr);
    glBindVertexArray(0);
}

shadow::ShaderType shadow::TextureMesh::getShaderType() const
{
    return ShaderType::Texture;
}
