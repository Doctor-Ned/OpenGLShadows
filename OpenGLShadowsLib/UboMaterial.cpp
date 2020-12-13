#include "UboMaterial.h"

#include <glm/gtc/type_ptr.hpp>

shadow::UboMaterial::UboMaterial() : UniformBufferObject("Material", 1)
{}

void shadow::UboMaterial::setAlbedo(glm::vec3& albedo)
{
    bufferSubData(value_ptr(albedo), sizeof(glm::vec3), offsetof(Material, albedo));
}

void shadow::UboMaterial::setRoughness(float roughness)
{
    bufferSubData(&roughness, sizeof(float), offsetof(Material, roughness));
}

void shadow::UboMaterial::setMetallic(float metallic)
{
    bufferSubData(&metallic, sizeof(float), offsetof(Material, metallic));
}
