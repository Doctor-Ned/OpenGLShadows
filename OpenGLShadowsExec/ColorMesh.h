#pragma once

#include "Vertex.h"
#include "Mesh.h"

#include "glad/glad.h"
#include <vector>

namespace shadow
{
    class ColorMesh final : public Mesh
    {
    public:
        ColorMesh(const std::vector<Vertex>& vertices, const std::vector<GLuint>& indices, glm::vec3 color);
        void setColor(glm::vec3 color);
        glm::vec3 getColor() const;
        void draw(std::shared_ptr<GLShader> shader) const override;
    private:
        glm::vec3 color{};
        GLuint vao{}, vbo{}, ebo{};
        GLsizei indexCount{};
    };
}

