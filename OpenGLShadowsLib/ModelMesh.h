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
        ModelMesh(ModelMesh&) = delete;
        ModelMesh(ModelMesh&&) = delete;
        ModelMesh& operator=(ModelMesh&) = delete;
        ModelMesh& operator=(ModelMesh&&) = delete;
        bool load();
        std::filesystem::path getPath() const;
        void draw(std::shared_ptr<GLShader> shader) const override;
        ShaderType getShaderType() const override;
    private:
        friend class ResourceManager;
        ModelMesh(std::filesystem::path path);
        void processNode(aiNode* node, const aiScene* scene);
        std::shared_ptr<TextureMesh> processMesh(aiMesh* mesh, const aiScene* scene) const;
        std::shared_ptr<Texture> loadTexture(TextureType textureType) const;
        std::vector<std::shared_ptr<TextureMesh>> meshes{};
        std::filesystem::path path{};
    };
}
