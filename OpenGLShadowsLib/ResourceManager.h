#pragma once

#include "Texture.h"
#include "ModelMesh.h"
#include "MaterialModelMesh.h"
#include "ModelData.h"
#include "ShaderManager.h"

#include <memory>
#include <filesystem>
#include <map>

namespace shadow
{

    class ResourceManager
    {
    public:
        ~ResourceManager();
        ResourceManager(ResourceManager&) = delete;
        ResourceManager(ResourceManager&&) = delete;
        ResourceManager& operator=(ResourceManager&) = delete;
        ResourceManager& operator=(ResourceManager&&) = delete;
        static ResourceManager& getInstance();
        bool initialize(std::filesystem::path resourceDirectory);
        bool reworkShaderFiles();
        void updateShaders() const;
        void updateVogelDisk(unsigned int shadowSamples, unsigned int penumbraSamples);
        std::string getShaderFileContent(const std::filesystem::path& path);
        std::shared_ptr<Texture> getTexture(const std::filesystem::path& path);
        std::shared_ptr<ModelMesh> getModel(const std::filesystem::path& path);
        std::shared_ptr<MaterialModelMesh> getMaterialModel(const std::filesystem::path& path, std::shared_ptr<Material> material);
        std::shared_ptr<GLShader> getShader(ShaderType shaderType);
        std::shared_ptr<UboMvp> getUboMvp() const;
        std::shared_ptr<UboMaterial> getUboMaterial() const;
        std::shared_ptr<UboLights> getUboLights() const;
        std::shared_ptr<UboWindow> getUboWindow() const;
        void renderQuad() const;
        static std::filesystem::path reworkPath(const std::filesystem::path& basePath, const std::filesystem::path& midPath, const std::filesystem::path& inputPath);
    private:
        ResourceManager() = default;
        std::shared_ptr<Texture> getTexture(const std::filesystem::path& path, bool shouldReworkPath);
        std::shared_ptr<ModelData> getModelData(const std::filesystem::path& path);
        void processModelNode(aiNode* node, const aiScene* scene, const std::filesystem::path& path, std::vector<ModelMeshData>& modelMeshData);
        shadow::ModelMeshData processModelMesh(aiMesh* mesh, const aiScene* scene, const std::filesystem::path& path);
        std::shared_ptr<shadow::Texture> loadModelTexture(TextureType textureType, const std::filesystem::path& path);
        bool initialised = false;
        const std::filesystem::path MODELS_TEXTURES_DIR{ "ModelsTextures" }, SHADERS_DIR{ "Shaders" };
        std::filesystem::path resourceDirectory{}, modelsTexturesDirectory{}, shadersDirectory{};
        std::map<std::filesystem::path, std::shared_ptr<Texture>> textures{};
        std::map<std::filesystem::path, std::shared_ptr<ModelData>> modelData{};
        GLuint quadVao{}, quadVbo{};
        std::unique_ptr<ShaderManager> shaderManager{};
    };
}
