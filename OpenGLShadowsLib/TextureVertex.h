#pragma once

#include <glm/glm.hpp>

namespace shadow
{
    struct TextureVertex
    {
        glm::vec3 position{};
        glm::vec3 normal{};
        glm::vec2 texCoords{};
        glm::vec3 tangent{};
        glm::vec3 bitangent{};
    };
}