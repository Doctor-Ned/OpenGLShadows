#pragma once

#include "UboLights.h"
#include "Framebuffer.h"

#include <memory>

namespace shadow
{
    class LightManager final
    {
    public:
        static LightManager& getInstance();
        bool initialize(GLsizei textureSize);
        void resize(GLsizei textureSize);
    private:
        LightManager() = default;
        std::shared_ptr<UboLights> uboLights{};
        GLsizei textureSize{};
        Framebuffer dirFbo{}, spotFbo{};
    };
}
