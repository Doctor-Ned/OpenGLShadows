#pragma once

#include <glm/glm.hpp>

namespace shadow
{
    struct Material
    {
        glm::vec3 albedo{};
        glm::vec3 roughness{};
        glm::vec3 metalness{};
    };
}