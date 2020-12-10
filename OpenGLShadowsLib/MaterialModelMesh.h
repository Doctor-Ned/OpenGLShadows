#pragma once

#include "MaterialMesh.h"

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

namespace shadow
{
    class MaterialModelMesh final : public Mesh
    {
    public:
        MaterialModelMesh(std::filesystem::path path, std::shared_ptr<Material> material);
        MaterialModelMesh(MaterialModelMesh&) = delete;
        MaterialModelMesh(MaterialModelMesh&&) = delete;
        MaterialModelMesh& operator=(MaterialModelMesh&) = delete;
        MaterialModelMesh& operator=(MaterialModelMesh&&) = delete;
        bool load();
        std::filesystem::path getPath() const;
        void draw(std::shared_ptr<GLShader> shader) const override;
        std::shared_ptr<Material> getMaterial() const;
        void setMaterial(std::shared_ptr<Material> material);
    private:
        void processNode(aiNode* node, const aiScene* scene);
        std::shared_ptr<MaterialMesh> processMesh(aiMesh* mesh, const aiScene* scene) const;
        std::vector<std::shared_ptr<MaterialMesh>> meshes{};
        std::shared_ptr<Material> material{};
        std::filesystem::path path{};
    };
}
