#include "DirectionalLight.h"
#include <glm/ext/matrix_clip_space.hpp>
#include <glm/ext/matrix_transform.hpp>

shadow::DirectionalLight::DirectionalLight(DirectionalLightData& data, float nearZ, float farZ)
    : DirectedLight<DirectionalLightData>(data, nearZ, farZ)
{}

glm::mat4 shadow::DirectionalLight::getLightSpaceMatrix()
{
    if (lightSpaceDirty)
    {
        lightSpaceDirty = false;
        float projSizeHalf = projectionSize * 0.5f;
        lightSpaceMatrix =
            glm::ortho(-projSizeHalf, projSizeHalf, -projSizeHalf, projSizeHalf, nearZ, farZ)
            * lookAt(position, position + lightData.direction, glm::vec3(0.0f, 1.0f, 0.0f));
    }
    return lightSpaceMatrix;
}

void shadow::DirectionalLight::setColor(glm::vec3& color)
{
    lightData.color = color;
    dirty = true;
}

void shadow::DirectionalLight::setStrength(float strength)
{
    lightData.strength = strength;
    dirty = true;
}

void shadow::DirectionalLight::setPosition(glm::vec3& position)
{
    this->position = position;
    lightSpaceDirty = true;
}

void shadow::DirectionalLight::setDirection(glm::vec3& direction)
{
    lightData.direction = normalize(direction);
    dirty = true;
    lightSpaceDirty = true;
}

void shadow::DirectionalLight::setProjectionSize(float projectionSize)
{
    this->projectionSize = projectionSize;
    lightSpaceDirty = true;
}
