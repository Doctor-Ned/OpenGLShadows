#include "LightManager.h"

shadow::LightManager::~LightManager()
{
    glDeleteBuffers(1, &dirSsbo);
    glDeleteBuffers(1, &spotSsbo);
}

shadow::LightManager& shadow::LightManager::getInstance()
{
    static LightManager lightManager{};
    return lightManager;
}

bool shadow::LightManager::initialize()
{
    assert(!dirSsbo);
    assert(!spotSsbo);
    SHADOW_DEBUG("Initializing LightManager...");
    dirSsbo = createSsbo(2);
    if (!dirSsbo)
    {
        SHADOW_ERROR("Failed to create DirectionalLight SSBO!");
        return false;
    }
    spotSsbo = createSsbo(3);
    if (!spotSsbo)
    {
        SHADOW_ERROR("Failed to create SpotLight SSBO!");
        return false;
    }

    SHADOW_DEBUG("LightManager initialized with SSBOs: {}, {}!", dirSsbo, spotSsbo);
    return true;
}

std::vector<std::shared_ptr<shadow::DirectionalLight>> shadow::LightManager::getDirectionalLights() const
{
    assert(dirSsbo);
    return directionalLights;
}

std::vector<std::shared_ptr<shadow::SpotLight>> shadow::LightManager::getSpotLights() const
{
    assert(spotSsbo);
    return spotLights;
}

void shadow::LightManager::addDirectionalLight(std::shared_ptr<DirectionalLight> directionalLight)
{
    assert(dirSsbo);
    const std::vector<std::shared_ptr<DirectionalLight>>::iterator it =
        std::find(directionalLights.begin(), directionalLights.end(), directionalLight);
    if (it == directionalLights.end())
    {
        directionalLights.push_back(directionalLight);
        SHADOW_DEBUG("Added directional light #{}!", directionalLights.size());
        std::vector<DirectionalLightData> data;
        for (std::shared_ptr<DirectionalLight>& light : directionalLights)
        {
            data.push_back(light->getData());
        }
        rebuildSsbo(dirSsbo, &data, data.size() * sizeof(DirectionalLightData));
    } else
    {
        SHADOW_WARN("Tried to add directional light that is already within LightManager!");
    }
}

void shadow::LightManager::addSpotLight(std::shared_ptr<SpotLight> spotLight)
{
    assert(spotSsbo);
    const std::vector<std::shared_ptr<SpotLight>>::iterator it =
        std::find(spotLights.begin(), spotLights.end(), spotLight);
    if (it == spotLights.end())
    {
        spotLights.push_back(spotLight);
        SHADOW_DEBUG("Added spot light #{}!", spotLights.size());
        std::vector<SpotLightData> data;
        for (std::shared_ptr<SpotLight>& light : spotLights)
        {
            data.push_back(light->getData());
        }
        rebuildSsbo(spotSsbo, &data, data.size() * sizeof(SpotLightData));
    } else
    {
        SHADOW_WARN("Tried to add spot light that is already within LightManager!");
    }
}

void shadow::LightManager::updateLights()
{
    for (size_t i = 0; i < directionalLights.size(); ++i)
    {
        if (directionalLights[i]->isDirty())
        {
            DirectionalLightData data = directionalLights[i]->getData();
            updateSsbo(dirSsbo, &data, sizeof(DirectionalLightData), i * sizeof(DirectionalLightData));
        }
    }

    for (size_t i = 0; i < spotLights.size(); ++i)
    {
        if (spotLights[i]->isDirty())
        {
            SpotLightData data = spotLights[i]->getData();
            updateSsbo(spotSsbo, &data, sizeof(SpotLightData), i * sizeof(SpotLightData));
        }
    }
}

GLuint shadow::LightManager::createSsbo(GLuint binding)
{
    GLuint ssbo;
    glGenBuffers(1, &ssbo);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo);
    glBufferData(GL_SHADER_STORAGE_BUFFER, 0, nullptr, GL_DYNAMIC_DRAW);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, binding, ssbo);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
    return ssbo;
}

void shadow::LightManager::rebuildSsbo(GLuint ssbo, void* data, GLsizeiptr size)
{
    assert(ssbo);
    SHADOW_DEBUG("Rebuilding SSBO {} to match size {}...", ssbo, size);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo);
    glBufferData(GL_SHADER_STORAGE_BUFFER, size, data, GL_DYNAMIC_DRAW);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
}

void shadow::LightManager::updateSsbo(GLuint ssbo, void* data, GLsizeiptr size, GLintptr offset)
{
    assert(ssbo);
    SHADOW_DEBUG("Updating SSBO {} with {} data at offset {}...", ssbo, size, offset);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo);
    glBufferSubData(GL_SHADER_STORAGE_BUFFER, offset, size, data);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
}
