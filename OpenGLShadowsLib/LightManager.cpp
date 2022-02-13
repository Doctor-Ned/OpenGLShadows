#include "LightManager.h"
#include "ResourceManager.h"

shadow::LightManager& shadow::LightManager::getInstance()
{
    static LightManager lightManager{};
    return lightManager;
}

#if SHADOW_MASTER || SHADOW_CHSS
bool shadow::LightManager::initialize(GLsizei textureSize, GLsizei penumbraTextureWidth, GLsizei penumbraTextureHeight)
{
    if (textureSize <= 0)
    {
        SHADOW_ERROR("Invalid texture size ({})!", textureSize);
        return false;
    }
    if (penumbraTextureWidth <= 0 || penumbraTextureHeight <= 0)
    {
        SHADOW_ERROR("Invalid penumbra texture size ({}x{})!", penumbraTextureWidth, penumbraTextureHeight);
        return false;
    }
    this->textureSize = textureSize;
    this->penumbraTextureWidth = penumbraTextureWidth;
    this->penumbraTextureHeight = penumbraTextureHeight;
    uboLights = ResourceManager::getInstance().getUboLights();
    if (!dirFbo.initialize(false, GL_DEPTH_ATTACHMENT, GL_DEPTH_COMPONENT,
        textureSize, textureSize, GL_DEPTH_COMPONENT, GL_FLOAT, GL_NEAREST, GL_CLAMP_TO_BORDER, glm::vec4(1.0f)))
    {
        return false;
    }
    if (!spotFbo.initialize(false, GL_DEPTH_ATTACHMENT, GL_DEPTH_COMPONENT,
        textureSize, textureSize, GL_DEPTH_COMPONENT, GL_FLOAT, GL_NEAREST, GL_CLAMP_TO_BORDER, glm::vec4(1.0f)))
    {
        return false;
    }
    if (!dirPenumbraFbo.initialize(true, GL_COLOR_ATTACHMENT0, GL_RED,
        penumbraTextureWidth, penumbraTextureHeight, GL_RED, GL_UNSIGNED_BYTE, GL_LINEAR, GL_CLAMP_TO_BORDER, glm::vec4(0.0f)))
    {
        return false;
    }
    if (!spotPenumbraFbo.initialize(true, GL_COLOR_ATTACHMENT0, GL_RED,
        penumbraTextureWidth, penumbraTextureHeight, GL_RED, GL_UNSIGNED_BYTE, GL_LINEAR, GL_CLAMP_TO_BORDER, glm::vec4(0.0f)))
    {
        return false;
    }
    return true;
}

void shadow::LightManager::resize(GLsizei textureSize, GLsizei penumbraTextureWidth, GLsizei penumbraTextureHeight)
{
    assert(textureSize > 0);
    assert(penumbraTextureWidth > 0);
    assert(penumbraTextureHeight > 0);
    if (this->textureSize != textureSize)
    {
        dirFbo.resize(textureSize, textureSize);
        spotFbo.resize(textureSize, textureSize);
        this->textureSize = textureSize;
    }
    if (this->penumbraTextureWidth != penumbraTextureWidth || this->penumbraTextureHeight != penumbraTextureHeight)
    {
        dirPenumbraFbo.resize(penumbraTextureWidth, penumbraTextureHeight);
        spotPenumbraFbo.resize(penumbraTextureWidth, penumbraTextureHeight);
        this->penumbraTextureWidth = penumbraTextureWidth;
        this->penumbraTextureHeight = penumbraTextureHeight;
    }
}
#else
bool shadow::LightManager::initialize(GLsizei textureSize)
{
    if (textureSize <= 0)
    {
        SHADOW_ERROR("Invalid texture size ({})!", textureSize);
        return false;
    }
    this->textureSize = textureSize;
    uboLights = ResourceManager::getInstance().getUboLights();
    if (!dirFbo.initialize(false, GL_DEPTH_ATTACHMENT, GL_DEPTH_COMPONENT,
        textureSize, textureSize, GL_DEPTH_COMPONENT, GL_FLOAT, GL_NEAREST, GL_CLAMP_TO_BORDER, glm::vec4(1.0f)))
    {
        return false;
    }
    if (!spotFbo.initialize(false, GL_DEPTH_ATTACHMENT, GL_DEPTH_COMPONENT,
        textureSize, textureSize, GL_DEPTH_COMPONENT, GL_FLOAT, GL_NEAREST, GL_CLAMP_TO_BORDER, glm::vec4(1.0f)))
    {
        return false;
    }
    return true;
}

void shadow::LightManager::resize(GLsizei textureSize)
{
    assert(textureSize > 0);
    if (this->textureSize != textureSize)
    {
        dirFbo.resize(textureSize, textureSize);
        spotFbo.resize(textureSize, textureSize);
        this->textureSize = textureSize;
    }
}
#endif
