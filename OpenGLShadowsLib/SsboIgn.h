#pragma once

#include "ShaderStorageBufferObject.h"

#include <glm/glm.hpp>

namespace shadow
{
    struct SsboIgnStruct
    {
        glm::uvec2 ignDimensions{};
        float* interleavedGradientNoise{};
    };

    class SsboIgn final : public ShaderStorageBufferObject<SsboIgnStruct>
    {
    public:
        SsboIgn(GLsizei windowWidth, GLsizei windowHeight);
        ~SsboIgn();
        SsboIgn(SsboIgn&) = delete;
        SsboIgn(SsboIgn&&) = delete;
        SsboIgn& operator=(SsboIgn&) = delete;
        SsboIgn& operator=(SsboIgn&&) = delete;
        void set(SsboIgnStruct& data) override;
        void resize(GLsizei windowWidth, GLsizei windowHeight);
    private:
        float getInterleavedGradientNoise(GLsizei x, GLsizei y, GLsizei windowWidth, GLsizei windowHeight) const;
        SsboIgnStruct dataStruct{};
    };
}
