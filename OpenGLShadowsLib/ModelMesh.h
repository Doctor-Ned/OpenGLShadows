#pragma once

#include "TextureMesh.h"
#include "ModelData.h"

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

namespace shadow
{
    class ModelMesh final : public Mesh
    {
    public:
        ModelMesh(ModelMesh&) = delete;
        ModelMesh(ModelMesh&&) = delete;
        ModelMesh& operator=(ModelMesh&) = delete;
        ModelMesh& operator=(ModelMesh&&) = delete;
        bool load();
        void draw(std::shared_ptr<GLShader> shader) const override;
        ShaderType getShaderType() const override;
    private:
        friend class ResourceManager;
        ModelMesh(std::shared_ptr<ModelData> modelData);
        std::vector<std::shared_ptr<TextureMesh>> meshes{};
        std::shared_ptr<ModelData> modelData{};
        std::filesystem::path path{};
    };
}
