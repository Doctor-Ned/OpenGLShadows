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

void shadow::TextureMesh::draw(std::shared_ptr<GLShader> shader) const
{
    shader->use();
    GLuint textureIndex = 0U;
    for (const std::pair<const TextureType, std::shared_ptr<Texture>> texture : textures)
    {
        glActiveTexture(GL_TEXTURE0 + textureIndex);
        switch (texture.first)
        {
            case TextureType::Albedo:
                shader->setInt("albedoTexture", textureIndex);
                break;
            case TextureType::Roughness:
                shader->setInt("roughnessTexture", textureIndex);
                break;
            case TextureType::Metalness:
                shader->setInt("metalnessTexture", textureIndex);
                break;
            case TextureType::Normal:
                shader->setInt("normalTexture", textureIndex);
                break;
            default:
                SHADOW_WARN("Encountered unsupported TextureType {}!", texture.first);
                continue;
        }
        glBindTexture(GL_TEXTURE_2D, texture.second->getId());
        ++textureIndex;
    }
    glActiveTexture(GL_TEXTURE0);
    glBindVertexArray(vao);
    glDrawElements(GL_TRIANGLES, indexCount, GL_UNSIGNED_INT, nullptr);
    glBindVertexArray(0);
}
