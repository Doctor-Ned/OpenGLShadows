#include "SpotLight.h"
#include "ShadowUtils.h"

#include <glm/ext/matrix_clip_space.hpp>
#include <glm/ext/matrix_transform.hpp>

glm::mat4 shadow::SpotLight::getLightSpaceMatrix()
{
    if (lightSpaceDirty)
    {
        lightSpaceDirty = false;
        lightSpaceMatrix =
            glm::perspective(FPI * 0.5f, 1.0f, nearZ, farZ)
            * lookAt(lightData.position, lightData.position + lightData.direction, glm::vec3(0.0f, 1.0f, 0.0f));
    }
    return lightSpaceMatrix;
}

void shadow::SpotLight::setColor(glm::vec3& color)
{
    lightData.color = color;
    dirty = true;
}

void shadow::SpotLight::setStrength(float strength)
{
    lightData.strength = strength;
    dirty = true;
}

void shadow::SpotLight::setDirection(glm::vec3& direction)
{
    lightData.direction = normalize(direction);
    dirty = true;
    lightSpaceDirty = true;
}

void shadow::SpotLight::setPosition(glm::vec3& position)
{
    lightData.position = position;
    dirty = true;
    lightSpaceDirty = true;
}

void shadow::SpotLight::setInnerCutOff(float innerCutOff)
{
    lightData.innerCutOff = innerCutOff;
    dirty = true;
}

void shadow::SpotLight::setOuterCutOff(float outerCutOff)
{
    lightData.outerCutOff = outerCutOff;
    dirty = true;
}
