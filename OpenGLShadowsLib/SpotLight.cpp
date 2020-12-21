#include "SpotLight.h"
#include "ShadowUtils.h"

#include <glm/ext/matrix_clip_space.hpp>
#include <glm/ext/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

shadow::SpotLight::SpotLight(SpotLightData& data, float nearZ, float farZ)
    : DirectedLight(data, nearZ, farZ)
{}

glm::mat4 shadow::SpotLight::getLightSpace()
{
    return lightData.lightSpace;
}

void shadow::SpotLight::updateLightSpace()
{
    lightData.lightSpace = glm::perspective(FPI * 0.5f, 1.0f, nearZ, farZ)
        * lookAt(lightData.position, lightData.position + lightData.direction, glm::vec3(0.0f, 1.0f, 0.0f));
    lightSpaceDirty = false;
}

void shadow::SpotLight::setColor(glm::vec3 color)
{
    lightData.color = color;
    dirty = true;
}

void shadow::SpotLight::setStrength(float strength)
{
    lightData.strength = strength;
    dirty = true;
}

void shadow::SpotLight::setDirection(glm::vec3 direction)
{
    lightData.direction = normalize(direction);
    dirty = true;
    lightSpaceDirty = true;
}

void shadow::SpotLight::setPosition(glm::vec3 position)
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

void shadow::SpotLight::drawGui()
{
    ImGui::PushID(this);
    ImGui::Text("Spot light");
    glm::vec3 color = lightData.color, position = lightData.position;
    float strength = lightData.strength;
    float aX = angleX, aY = angleY, aZ = angleZ;
    float innerAngle = std::acosf(lightData.innerCutOff), outerAngle = std::acosf(lightData.outerCutOff);
    float nearZ = this->nearZ, farZ = this->farZ;
    ImGui::ColorPicker3("Color", value_ptr(color));
    ImGui::DragFloat("Near Z", &nearZ, 0.1f);
    ImGui::DragFloat("Far Z", &farZ, 0.1f);
    ImGui::SliderFloat("Strength", &strength, 0.0f, 10.0f);
    ImGui::DragFloat3("Position", value_ptr(position), 0.1f);
    ImGui::SliderAngle("Angle X", &aX);
    ImGui::SliderAngle("Angle Y", &aY);
    ImGui::SliderAngle("Angle Z", &aZ);
    glm::vec3 direction =
        glm::quat(glm::vec3(aX, aY, aZ)) * glm::vec3(0.0f, 0.0f, -1.0f);
    ImGui::TextWrapped("Direction: [%.1f, %.1f, %.1f]", direction[0], direction[1], direction[2]);
    ImGui::SliderAngle("Inner cutoff angle", &innerAngle, 0.0f, 180.0f);
    ImGui::SliderAngle("Outer cutoff angle", &outerAngle, 0.0f, 180.0f);
    if (nearZ != this->nearZ)
    {
        setNearZ(nearZ);
    }
    if (farZ != this->farZ)
    {
        setFarZ(farZ);
    }
    if (color != lightData.color)
    {
        setColor(color);
    }
    if (position != lightData.position)
    {
        setPosition(position);
    }
    if (direction != lightData.direction)
    {
        angleX = aX;
        angleY = aY;
        angleZ = aZ;
        setDirection(direction);
    }
    if (strength != lightData.strength)
    {
        setStrength(strength);
    }
    float innerCutOff = std::cosf(innerAngle), outerCutOff = std::cosf(outerAngle);
    if (std::abs(innerCutOff - lightData.innerCutOff) > 0.001f)
    {
        setInnerCutOff(innerCutOff);
    }
    if (std::abs(outerCutOff - lightData.outerCutOff) > 0.001f)
    {
        setOuterCutOff(outerCutOff);
    }
    ImGui::PopID();
}
