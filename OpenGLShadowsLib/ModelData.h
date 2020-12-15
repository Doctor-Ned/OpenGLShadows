#pragma once

#include "ShadowLog.h"
#include "TextureType.h"
#include "Texture.h"

#include "glad/glad.h"
#include <glm/glm.hpp>
#include <vector>
#include <map>

namespace shadow
{
    struct ModelMeshData final
    {
        std::vector<glm::vec3> vertices{};
        std::vector<glm::vec3> normals{};
        std::vector<glm::vec2> texCoords{};
        std::vector<glm::vec3> tangents{};
        std::vector<glm::vec3> bitangents{};
        std::vector<GLuint> indices{};
        std::map<TextureType, std::shared_ptr<Texture>> textures{};
    };

    struct ModelData final
    {
        ModelData(std::vector<ModelMeshData> modelMeshData) : modelMeshData(std::move(modelMeshData)) {}
        std::vector<ModelMeshData> modelMeshData{};
    };
}