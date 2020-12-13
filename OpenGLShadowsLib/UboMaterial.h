#pragma once

#include "UniformBufferObject.h"
#include "Material.h"

#include <glm/glm.hpp>

namespace shadow
{
    class UboMaterial final : public UniformBufferObject<Material>
    {
    public:
        UboMaterial();
        void setAlbedo(glm::vec3& albedo);
        void setRoughness(float roughness);
        void setMetallic(float metallic);
    };
}