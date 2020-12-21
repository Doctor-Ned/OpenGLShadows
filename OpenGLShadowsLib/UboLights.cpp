#include "UboLights.h"

shadow::UboLights::UboLights(std::shared_ptr<DirectionalLight> directionalLight, std::shared_ptr<SpotLight> spotLight)
    : UniformBufferObject("Lights", 2), directionalLight(directionalLight), spotLight(spotLight)
{
    assert(directionalLight);
    assert(spotLight);
}

void shadow::UboLights::update()
{
    bool dirty = directionalLight->isLightSpaceDirty();
    if (dirty)
    {
        directionalLight->updateLightSpace();
    }
    if (dirty || directionalLight->isDirty())
    {
        bufferSubData(&(directionalLight->getData()), sizeof(DirectionalLightData), offsetof(Lights, dirLightData));
    }
    dirty = spotLight->isLightSpaceDirty();
    if (dirty)
    {
        spotLight->updateLightSpace();
    }
    if (dirty || spotLight->isDirty())
    {
        bufferSubData(&(spotLight->getData()), sizeof(SpotLightData), offsetof(Lights, spotLightData));
    }
}

std::shared_ptr<shadow::DirectionalLight> shadow::UboLights::getDirectionalLight() const
{
    return directionalLight;
}

std::shared_ptr<shadow::SpotLight> shadow::UboLights::getSpotLight() const
{
    return spotLight;
}

void shadow::UboLights::setAmbient(float ambient)
{
    bufferSubData(&ambient, sizeof(float), offsetof(Lights, ambient));
}
