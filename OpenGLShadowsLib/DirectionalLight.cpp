#include "DirectionalLight.h"
#include <glm/ext/matrix_clip_space.hpp>
#include <glm/ext/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

shadow::DirectionalLight::DirectionalLight(DirectionalLightData& data, float nearZ, float farZ)
    : DirectedLight<DirectionalLightData>(data, nearZ, farZ)
{}

glm::mat4 shadow::DirectionalLight::getLightSpace()
{
    return lightData.lightSpace;
}

void shadow::DirectionalLight::updateLightSpace()
{
    float projSizeHalf = projectionSize * 0.5f;
    lightData.lightSpace =
        glm::ortho(-projSizeHalf, projSizeHalf, -projSizeHalf, projSizeHalf, nearZ, farZ)
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

void shadow::DirectionalLight::setProjectionSize(float projectionSize)
{
    this->projectionSize = projectionSize;
    lightSpaceDirty = true;
}

void shadow::DirectionalLight::drawGui()
{
    ImGui::PushID(this);
    ImGui::Text("Directional light");
    glm::vec3 color = lightData.color;
    glm::vec3 position = this->position;
    float strength = lightData.strength;
    float aX = angleX, aY = angleY, aZ = angleZ;
    float nearZ = this->nearZ, farZ = this->farZ, projectionSize = this->projectionSize;
    ImGui::ColorPicker3("Color", value_ptr(color));
    ImGui::DragFloat("Near Z", &nearZ, 0.1f);
    ImGui::DragFloat("Far Z", &farZ, 0.1f);
    ImGui::DragFloat("Projection size", &projectionSize, 0.25f);
    ImGui::DragFloat3("Position", value_ptr(position), 0.25f);
    ImGui::SliderFloat("Strength", &strength, 0.0f, 10.0f);
    ImGui::SliderAngle("Angle X", &aX);
    ImGui::SliderAngle("Angle Y", &aY);
    ImGui::SliderAngle("Angle Z", &aZ);
    glm::vec3 direction =
        glm::quat(glm::vec3(aX, aY, aZ)) * glm::vec3(0.0f, 0.0f, -1.0f);
    ImGui::TextWrapped("Direction: [%.1f, %.1f, %.1f]", direction[0], direction[1], direction[2]);
    if (nearZ != this->nearZ)
    {
        setNearZ(nearZ);
    }
    if (farZ != this->farZ)
    {
        setFarZ(farZ);
    }
    if (projectionSize != this->projectionSize)
    {
        setProjectionSize(projectionSize);
    }
    if (position != this->position)
    {
        setPosition(position);
    }
    if (color != lightData.color)
    {
        setColor(color);
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
    ImGui::PopID();
}
