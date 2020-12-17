#pragma once

#include <glm/glm.hpp>

namespace shadow
{
    struct Vertex2D final
    {
        glm::vec2 position{};
        glm::vec2 texCoords{};
    };
}