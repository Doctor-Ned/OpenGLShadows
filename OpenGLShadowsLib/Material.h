#pragma once

#include <glm/glm.hpp>

namespace shadow
{
    struct Material
    {
        glm::vec3 albedo{};
        float roughness{};
        float metallic{};
    };
}