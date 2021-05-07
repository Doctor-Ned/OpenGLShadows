#pragma once

#include "UniformBufferObject.h"

#include <glm/glm.hpp>

namespace shadow
{
    struct UboWindowStruct
    {
        glm::vec2 windowSize{};
    };

    class UboWindow final : public UniformBufferObject<UboWindowStruct>
    {
    public:
        UboWindow();
        void setWindowSize(glm::vec2& windowSize);
    };
}