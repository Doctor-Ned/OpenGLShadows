#pragma once

#include "UniformBufferObject.h"

#include <glm/glm.hpp>

namespace shadow
{
    struct ModelViewProjection
    {
        glm::mat4 model{};
        glm::mat4 view{};
        glm::mat4 projection{};
    };

    class UboModelViewProjection final : UniformBufferObject<ModelViewProjection>
    {
    public:
        UboModelViewProjection();
        void setModel(glm::mat4& model);
        void setView(glm::mat4& view);
        void setProjection(glm::mat4& projection);
    };
}