#pragma once

#include <glm/glm.hpp>

namespace shadow
{
    struct Material
    {
        Material(glm::vec3 albedo, float roughness, float metallic)
        : albedo(std::move(albedo)), roughness(roughness), metallic(metallic) {}
        glm::vec3 albedo{};
        float roughness{};
        float metallic{};
    };
}