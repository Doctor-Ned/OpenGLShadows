#include "DirectionalLight.h"
#include <glm/ext/matrix_clip_space.hpp>
#include <glm/ext/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

shadow::DirectionalLight::DirectionalLight(DirectionalLightData& data) : DirectedLight<DirectionalLightData>(data)
{}

glm::mat4 shadow::DirectionalLight::getLightSpace()
{
    return lightData.lightSpace;
}

void shadow::DirectionalLight::updateLightSpace()
{
    float projSizeHalf = projectionSize * 0.5f;
    lightData.lightSpace =
        glm::ortho(-projSizeHalf, projSizeHalf, -projSizeHalf, projSizeHalf, lightData.nearZ, lightData.farZ)
        * lookAt(position, position + lightData.direction, glm::vec3(0.0f, 1.0f, 0.0f));
    lightSpaceDirty = false;
}

void shadow::DirectionalLight::setColor(glm::vec3 color)
{
    lightData.color = color;
    dirty = true;
}

void shadow::DirectionalLight::setStrength(float strength)
{
    lightData.strength = strength;
    dirty = true;
}

void shadow::DirectionalLight::setPosition(glm::vec3 position)
{
    this->position = position;
    lightSpaceDirty = true;
}

void shadow::DirectionalLight::setDirection(glm::vec3 direction)
{
    lightData.direction = normalize(direction);
    dirty = true;
    lightSpaceDirty = true;
}



void shadow::DirectionalLight::setNearZ(float nearZ)
{
    lightData.nearZ = nearZ;
    dirty = lightSpaceDirty = true;
}

void shadow::DirectionalLight::setFarZ(float farZ)
{
    lightData.farZ = farZ;
    dirty = lightSpaceDirty = true;
}

void shadow::DirectionalLight::setLightSize(float lightSize)
{
    lightData.lightSize = lightSize;
    dirty = true;
}

void shadow::DirectionalLight::setProjectionSize(float projectionSize)
{
    this->projectionSize = projectionSize;
    lightSpaceDirty = true;
}

float shadow::DirectionalLight::getProjectionSize() const
{
    return projectionSize;
}
