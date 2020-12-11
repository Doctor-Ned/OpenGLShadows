#include "PrimitiveData.h"
#include "ShadowUtils.h"

shadow::PrimitiveData::PrimitiveData(const std::vector<glm::vec3>& vertices, const std::vector<glm::vec2>& texCoords,
                                     const std::vector<GLuint>& indices) : vertices(vertices), texCoords(texCoords), indices(indices)
{}

bool shadow::PrimitiveData::isValid() const
{
    if (vertices.empty())
    {
        SHADOW_ERROR("No vertices provided!");
        return false;
    }
    if (indices.empty())
    {
        SHADOW_ERROR("No indices provided!");
        return false;
    }
    if (vertices.size() != texCoords.size())
    {
        SHADOW_ERROR("The amount of vertices ({}) is not equal to the amount of texture coords ({})!", vertices.size(), texCoords.size());
        return false;
    }
    if (indices.size() % 3)
    {
        SHADOW_ERROR("The amount of indices ({}) is not a multiple of 3!", indices.size());
        return false;
    }
    return true;
}

std::vector<shadow::TextureVertex> shadow::PrimitiveData::toTextureVertex() const
{
    if (!isValid())
    {
        return {};
    }
    std::vector<TextureVertex> result{};
    std::vector<glm::vec3> normals{ ShadowUtils::generateNormals(vertices, indices) };
    assert(vertices.size() == normals.size());
    for (unsigned int i = 0; i < vertices.size(); ++i)
    {
        result.push_back({ vertices[i], normals[i], texCoords[i] });
    }
    return result;
}

std::vector<shadow::Vertex> shadow::PrimitiveData::toVertex() const
{
    if (!isValid())
    {
        return {};
    }
    std::vector<Vertex> result{};
    std::vector<glm::vec3> normals{ ShadowUtils::generateNormals(vertices, indices) };
    assert(vertices.size() == normals.size());
    for (unsigned int i = 0; i < vertices.size(); ++i)
    {
        result.push_back({ vertices[i], normals[i] });
    }
    return result;
}

const std::vector<GLuint>& shadow::PrimitiveData::getIndices() const
{
    return indices;
}
