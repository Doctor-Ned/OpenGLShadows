#pragma once

#include "Vertex.h"
#include "Mesh.h"
#include "Material.h"
#include "PrimitiveData.h"

#include "glad/glad.h"
#include <vector>

namespace shadow
{
    class MaterialMesh final : public Mesh
    {
    public:
        MaterialMesh(const std::vector<Vertex>& vertices, const std::vector<GLuint>& indices, std::shared_ptr<Material> material);
        static std::shared_ptr<MaterialMesh> fromPrimitiveData(std::shared_ptr<PrimitiveData> data, std::shared_ptr<Material> material);
        void setMaterial(std::shared_ptr<Material> material);
        std::shared_ptr<Material> getMaterial() const;
        void draw(std::shared_ptr<GLShader> shader) const override;
    private:
        std::shared_ptr<Material> material{};
        GLuint vao{}, vbo{}, ebo{};
        GLsizei indexCount{};
    };
}

