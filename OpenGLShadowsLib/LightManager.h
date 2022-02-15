#pragma once

#include "UboLights.h"
#include "Framebuffer.h"
#include "ShadowVariants.h"

#include <memory>

namespace shadow
{
    class LightManager final
    {
    public:
        static LightManager& getInstance();
        inline GLsizei getTextureSize() const;
#if SHADOW_MASTER || SHADOW_CHSS
        bool initialize(GLsizei textureSize, GLsizei penumbraTextureWidth, GLsizei penumbraTextureHeight);
        void resize(GLsizei textureSize, GLsizei penumbraTextureWidth, GLsizei penumbraTextureHeight);
        inline GLsizei getPenumbraTextureWidth() const;
        inline GLsizei getPenumbraTextureHeight() const;
        inline GLuint getDirPenumbraFbo() const;
        inline GLuint getSpotPenumbraFbo() const;
        inline GLuint getDirPenumbraTexture() const;
        inline GLuint getSpotPenumbraTexture() const;
#else
        bool initialize(GLsizei textureSize);
        void resize(GLsizei textureSize);
#endif
#if SHADOW_VSM
        inline GLuint getTempFbo() const;
        inline GLuint getTempTexture() const;
#endif
        inline GLuint getDirFbo() const;
        inline GLuint getSpotFbo() const;
        inline GLuint getDirTexture() const;
        inline GLuint getSpotTexture() const;
    private:
        LightManager() = default;
        std::shared_ptr<UboLights> uboLights{};
        Framebuffer dirFbo{}, spotFbo{};
        GLsizei textureSize{};
#if SHADOW_MASTER || SHADOW_CHSS
        GLsizei penumbraTextureWidth{}, penumbraTextureHeight{};
        Framebuffer dirPenumbraFbo{}, spotPenumbraFbo{};
#elif SHADOW_VSM
        Framebuffer tempFbo{};
#endif
    };

    inline GLsizei LightManager::getTextureSize() const
    {
        assert(textureSize);
        return textureSize;
    }

#if SHADOW_MASTER || SHADOW_CHSS
    inline GLsizei LightManager::getPenumbraTextureWidth() const
    {
        assert(penumbraTextureWidth);
        return penumbraTextureWidth;
    }

    inline GLsizei LightManager::getPenumbraTextureHeight() const
    {
        assert(penumbraTextureHeight);
        return penumbraTextureHeight;
    }

    inline GLuint LightManager::getDirPenumbraFbo() const
    {
        return dirPenumbraFbo.getFbo();
    }

    inline GLuint LightManager::getSpotPenumbraFbo() const
    {
        return spotPenumbraFbo.getFbo();
    }

    inline GLuint LightManager::getDirPenumbraTexture() const
    {
        return dirPenumbraFbo.getTexture();
    }

    inline GLuint LightManager::getSpotPenumbraTexture() const
    {
        return spotPenumbraFbo.getTexture();
    }
#endif

#if SHADOW_VSM
    inline GLuint LightManager::getTempFbo() const
    {
        return tempFbo.getFbo();
    }

    inline GLuint LightManager::getTempTexture() const
    {
        return tempFbo.getTexture();
    }
#endif

    inline GLuint LightManager::getDirFbo() const
    {
        return dirFbo.getFbo();
    }

    inline GLuint LightManager::getSpotFbo() const
    {
        return spotFbo.getFbo();
    }

    inline GLuint LightManager::getDirTexture() const
    {
        return dirFbo.getTexture();
    }

    inline GLuint LightManager::getSpotTexture() const
    {
        return spotFbo.getTexture();
    }
}
