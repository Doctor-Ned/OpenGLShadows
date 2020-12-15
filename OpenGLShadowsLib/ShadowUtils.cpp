#include "ShadowUtils.h"

std::vector<glm::vec3> shadow::ShadowUtils::generateNormals(const std::vector<glm::vec3>& vertices, const std::vector<GLuint>& indices)
{
    std::vector<glm::vec3> result(vertices.size(), { 0.0f, 0.0f, 0.0f });
    assert(indices.size() % 3 == 0);
    for (unsigned int i = 0; i < indices.size(); ++i)
    {
        GLuint i1 = indices[i], i2 = indices[++i], i3 = indices[++i];
        assert(max3(i1, i2, i3) < vertices.size());
        glm::vec3 normal = getNormal(vertices[i1], vertices[i2], vertices[i3]);
        result[i1] += normal;
        result[i2] += normal;
        result[i3] += normal;
    }
    for (glm::vec3& normal : result)
    {
        normal = normalize(normal);
    }
    return result;
}

glm::vec3 shadow::ShadowUtils::getNormal(const glm::vec3& v1, const glm::vec3& v2, const glm::vec3& v3)
{
    return normalize(cross(normalize(v3 - v1), normalize(v2 - v1)));
}

void shadow::ShadowUtils::generateTangentsBitangents(std::vector<TextureVertex>& vert, const std::vector<GLuint>& indices)
{
    assert(!indices.empty());
    assert(indices.size() % 3 == 0);
    for (size_t i = 0; i < indices.size(); ++i)
    {
        GLuint i1 = indices[i], i2 = indices[++i], i3 = indices[++i];
        TextureVertex& v1 = vert[i1], &v2 = vert[i2], &v3 = vert[i3];
        glm::vec3 edge1 = v2.position - v1.position;
        glm::vec3 edge2 = v3.position - v1.position;
        glm::vec2 deltaUV1 = v2.texCoords - v1.texCoords;
        glm::vec2 deltaUV2 = v3.texCoords - v1.texCoords;
        float f = 1.0f / (deltaUV1.x * deltaUV2.y - deltaUV2.x * deltaUV1.y);
        glm::vec3 tangent = (edge1 * deltaUV2.y - edge2 * deltaUV1.y) * f;
        glm::vec3 bitangent = (deltaUV1.x * edge2 - deltaUV2.x * edge1) * f;
        v1.tangent += tangent;
        v2.tangent += tangent;
        v3.tangent += tangent;
        v1.bitangent += bitangent;
        v2.bitangent += bitangent;
        v3.bitangent += bitangent;
    }
    for (TextureVertex& vertex : vert)
    {
        vertex.tangent = normalize(vertex.tangent);
        vertex.bitangent = normalize(vertex.bitangent);
    }
}
