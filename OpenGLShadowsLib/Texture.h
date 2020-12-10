#pragma once

#include "glad/glad.h"
#include <filesystem>
#include <cassert>

namespace shadow
{
    enum class TextureType
    {
        Albedo,
        Roughness,
        Metalness,
        Normal
    };

    class Texture final
    {
    public:
        ~Texture();
        Texture(Texture&) = delete;
        Texture(Texture&&) = delete;
        Texture& operator=(Texture&) = delete;
        Texture& operator=(Texture&&) = delete;
        bool load();
        inline GLuint getId() const;
        int getWidth() const;
        int getHeight() const;
        std::filesystem::path getPath() const;
    private:
        friend class ResourceManager;
        Texture(std::filesystem::path path);
        int width{}, height{};
        std::filesystem::path path{};
        GLuint textureId{};
    };

    inline GLuint Texture::getId() const
    {
        assert(textureId);
        return textureId;
    }
}

