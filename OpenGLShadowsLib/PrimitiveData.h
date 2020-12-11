#pragma once

#include "Vertex.h"
#include "TextureVertex.h"
#include "ShadowLog.h"

#include "glad/glad.h"

namespace shadow
{
    class PrimitiveData final
    {
    public:
        PrimitiveData(const std::vector<glm::vec3>& vertices, const std::vector<glm::vec2>& texCoords, const std::vector<GLuint>& indices);
        bool isValid() const;
        std::vector<TextureVertex> toTextureVertex() const;
        std::vector<Vertex> toVertex() const;
        const std::vector<GLuint>& getIndices() const;
    private:
        std::vector<glm::vec3> vertices{};
        std::vector<glm::vec2> texCoords{};
        std::vector<GLuint> indices{};
    };
}