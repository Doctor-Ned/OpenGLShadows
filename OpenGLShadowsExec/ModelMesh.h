#pragma once

#include "TextureMesh.h"

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

namespace shadow
{
    class ModelMesh final : public Mesh
    {
    public:
        ModelMesh(std::filesystem::path path);
        ModelMesh(ModelMesh&) = delete;
        ModelMesh(ModelMesh&&) = delete;
        ModelMesh& operator=(ModelMesh&) = delete;
        ModelMesh& operator=(ModelMesh&&) = delete;
        bool load();
        std::filesystem::path getPath() const;
        void draw(std::shared_ptr<GLShader> shader) const override;
    private:
        void processNode(aiNode* node, const aiScene* scene);
        std::shared_ptr<TextureMesh> processMesh(aiMesh* mesh, const aiScene* scene) const;
        std::shared_ptr<Texture> loadFirstTexture(aiMaterial* mat, aiTextureType type) const;
        std::vector<std::shared_ptr<TextureMesh>> meshes{};
        std::filesystem::path path{};
    };
}
