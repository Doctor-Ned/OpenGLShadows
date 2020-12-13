#pragma once

#include "MaterialMesh.h"
#include "ModelData.h"

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

namespace shadow
{
    class MaterialModelMesh final : public Mesh
    {
    public:
        MaterialModelMesh(MaterialModelMesh&) = delete;
        MaterialModelMesh(MaterialModelMesh&&) = delete;
        MaterialModelMesh& operator=(MaterialModelMesh&) = delete;
        MaterialModelMesh& operator=(MaterialModelMesh&&) = delete;
        bool load();
        void draw(std::shared_ptr<GLShader> shader) const override;
        ShaderType getShaderType() const override;
        std::shared_ptr<Material> getMaterial() const;
        void setMaterial(std::shared_ptr<Material> material);
    private:
        friend class ResourceManager;
        MaterialModelMesh(std::shared_ptr<ModelData> modelData, std::shared_ptr<Material> material);
        std::vector<std::shared_ptr<MaterialMesh>> meshes{};
        std::shared_ptr<ModelData> modelData{};
        std::shared_ptr<Material> material{};
        std::shared_ptr<UboMaterial> uboMaterial{};
    };
}
