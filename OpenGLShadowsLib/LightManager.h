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
        bool initialize(GLsizei textureSize, GLsizei penumbraTextureWidth, GLsizei penumbraTextureHeight);
        void resize(GLsizei textureSize, GLsizei penumbraTextureWidth, GLsizei penumbraTextureHeight);
        inline GLsizei getTextureSize() const;
        inline GLsizei getPenumbraTextureWidth() const;
        inline GLsizei getPenumbraTextureHeight() const;
        inline GLuint getDirFbo() const;
        inline GLuint getDirPenumbraFbo() const;
        inline GLuint getSpotFbo() const;
        inline GLuint getSpotPenumbraFbo() const;
        inline GLuint getDirTexture() const;
        inline GLuint getDirPenumbraTexture() const;
        inline GLuint getSpotTexture() const;
        inline GLuint getSpotPenumbraTexture() const;
    private:
        LightManager() = default;
        std::shared_ptr<UboLights> uboLights{};
        GLsizei textureSize{}, penumbraTextureWidth{}, penumbraTextureHeight{};
        Framebuffer dirFbo{}, spotFbo{}, dirPenumbraFbo{}, spotPenumbraFbo{};
    };

    inline GLsizei LightManager::getTextureSize() const
    {
        assert(textureSize);
        return textureSize;
    }

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

    inline GLuint LightManager::getDirFbo() const
    {
        return dirFbo.getFbo();
    }

    inline GLuint LightManager::getDirPenumbraFbo() const
    {
        return dirPenumbraFbo.getFbo();
    }

    inline GLuint LightManager::getSpotFbo() const
    {
        return spotFbo.getFbo();
    }

    inline GLuint LightManager::getSpotPenumbraFbo() const
    {
        return spotPenumbraFbo.getFbo();
    }

    inline GLuint LightManager::getDirTexture() const
    {
        return dirFbo.getTexture();
    }

    inline GLuint LightManager::getDirPenumbraTexture() const
    {
        return dirPenumbraFbo.getTexture();
    }

    inline GLuint LightManager::getSpotTexture() const
    {
        return spotFbo.getTexture();
    }

    inline GLuint LightManager::getSpotPenumbraTexture() const
    {
        return spotPenumbraFbo.getTexture();
    }
}
