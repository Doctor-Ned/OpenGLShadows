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
        glm::vec3 viewPosition{};
    };

    class UboModelViewProjection final : public UniformBufferObject<ModelViewProjection>
    {
    public:
        UboModelViewProjection();
        void setModel(glm::mat4& model);
        void setView(glm::mat4& view);
        void setProjection(glm::mat4& projection);
        void setViewPosition(glm::vec3& viewPosition);
    };
}