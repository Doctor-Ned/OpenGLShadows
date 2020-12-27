#include "LightManager.h"
#include "ResourceManager.h"

shadow::LightManager& shadow::LightManager::getInstance()
{
    static LightManager lightManager{};
    return lightManager;
}

bool shadow::LightManager::initialize(GLsizei textureSize)
{
    if (textureSize <= 0)
    {
        SHADOW_ERROR("Invalid texture size ({})!", textureSize);
        return false;
    }
    this->textureSize = textureSize;
    uboLights = ResourceManager::getInstance().getUboLights();
    if (!dirFbo.initialize(true, GL_COLOR_ATTACHMENT0, GL_RG,
                           textureSize, textureSize, GL_RG, GL_FLOAT, GL_LINEAR, GL_CLAMP_TO_BORDER, glm::vec4(1.0f)))
    {
        return false;
    }
    if (!spotFbo.initialize(true, GL_COLOR_ATTACHMENT0, GL_RG,
                            textureSize, textureSize, GL_RG, GL_FLOAT, GL_LINEAR, GL_CLAMP_TO_BORDER, glm::vec4(1.0f)))
    {
        return false;
    }
    if (!tempFbo.initialize(false, GL_COLOR_ATTACHMENT0, GL_RG,
                            textureSize, textureSize, GL_RG, GL_FLOAT, GL_NEAREST, GL_CLAMP_TO_BORDER, glm::vec4(1.0f)))
    {
        return false;
    }
    return true;
}

void shadow::LightManager::resize(GLsizei textureSize)
{
    assert(textureSize > 0);
    if (this->textureSize == textureSize)
    {
        return;
    }
    dirFbo.resize(textureSize, textureSize);
    spotFbo.resize(textureSize, textureSize);
    tempFbo.resize(textureSize, textureSize);
    this->textureSize = textureSize;
}
