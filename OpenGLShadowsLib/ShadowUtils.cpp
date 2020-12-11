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
    return normalize(cross(v3 - v1, v2 - v1));
}
