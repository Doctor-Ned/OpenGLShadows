#include "Texture.h"
#include "ShadowLog.h"

#include <utility>
#include <stb_image.h>

shadow::Texture::Texture(std::filesystem::path path) : path(std::move(path)) {}

shadow::Texture::~Texture()
{
    if (textureId)
    {
        glDeleteTextures(1, &textureId);
        textureId = 0U;
    }
}

bool shadow::Texture::load()
{
    if (textureId)
    {
        return true;
    }
    SHADOW_DEBUG("Loading texture '{}'...", path.generic_string());
    if (!exists(path))
    {
        SHADOW_ERROR("Texture file '{}' does not exist!", path.generic_string());
        return false;
    }
    stbi_set_flip_vertically_on_load(true);
    int comp;
    unsigned char* data = stbi_load(path.generic_string().c_str(), &width, &height, &comp, 0);
    if (!data)
    {
        SHADOW_ERROR("Failed to load texture '{}'!", path.generic_string());
        return false;
    }
    GLenum format;
    switch (comp)
    {
        case 1:
            format = GL_RED;
            break;
        case 2:
            format = GL_RG;
            break;
        case 3:
            format = GL_RGB;
            break;
        case 4:
            format = GL_RGBA;
            break;
        default:
        {
            stbi_image_free(data);
            SHADOW_ERROR("Texture '{}' uses an unknown {}-component format!", path.generic_string(), comp);
            return false;
        }
    }
    glGenTextures(1, &textureId);
    glBindTexture(GL_TEXTURE_2D, textureId);
    glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
    glGenerateMipmap(GL_TEXTURE_2D);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    stbi_image_free(data);
    return true;
}

int shadow::Texture::getWidth() const
{
    return width;
}

int shadow::Texture::getHeight() const
{
    return height;
}

std::filesystem::path shadow::Texture::getPath() const
{
    return path;
}
