#pragma once

#include "Texture.h"
#include "ModelMesh.h"
#include "MaterialModelMesh.h"
#include "ModelData.h"
#include "UboMvp.h"
#include "UboMaterial.h"
#include "UboLights.h"

#include <memory>
#include <filesystem>
#include <map>

namespace shadow
{
    struct ShaderFileInfo final
    {
        std::vector<std::filesystem::path> references{}; // todo: this could actually be a set of some sort
        std::filesystem::file_time_type timestamp{};
        std::string content{};
        bool modified{};
    };

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
        void reworkShaderFiles();
        void updateShaders() const;
        std::shared_ptr<Texture> getTexture(const std::filesystem::path& path);
        std::shared_ptr<ModelMesh> getModel(const std::filesystem::path& path);
        std::shared_ptr<MaterialModelMesh> getMaterialModel(const std::filesystem::path& path, std::shared_ptr<Material> material);
        std::shared_ptr<GLShader> getShader(ShaderType shaderType);
        std::shared_ptr<UboMvp> getUboMvp() const;
        std::shared_ptr<UboMaterial> getUboMaterial() const;
        std::shared_ptr<UboLights> getUboLights() const;
        void renderQuad() const;
    private:
        ResourceManager() = default;
        std::shared_ptr<Texture> getTexture(const std::filesystem::path& path, bool shouldReworkPath);
        std::shared_ptr<ModelData> getModelData(const std::filesystem::path& path);
        void processModelNode(aiNode* node, const aiScene* scene, const std::filesystem::path& path, std::vector<ModelMeshData>& modelMeshData);
        shadow::ModelMeshData processModelMesh(aiMesh* mesh, const aiScene* scene, const std::filesystem::path& path);
        std::shared_ptr<shadow::Texture> loadModelTexture(TextureType textureType, const std::filesystem::path& path);
        bool rebuildShaderFile(const std::filesystem::path& path);
        bool isShaderFileRecursivelyReferenced(const std::filesystem::path& path, const std::filesystem::path& searchPath);
        bool isShaderFileModified(const std::filesystem::path& path);
        void loadShaders();
        static std::filesystem::path reworkPath(const std::filesystem::path& basePath, const std::filesystem::path& midPath, const std::filesystem::path& inputPath);
        bool initialised = false;
        const std::filesystem::path MODELS_TEXTURES_DIR{ "ModelsTextures" }, SHADERS_DIR{ "Shaders" };
        const char* INCLUDE_TEXT = "#include ", * INCLUDED_FROM_TEXT = "#includedfrom ", * END_INCLUDE_TEXT = "#endinclude";
        const size_t INCLUDE_LENGTH = strlen(INCLUDE_TEXT), INCLUDED_FROM_LENGTH = strlen(INCLUDED_FROM_TEXT), END_INCLUDE_LENGTH = strlen(END_INCLUDE_TEXT);
        std::filesystem::path resourceDirectory{}, modelsTexturesDirectory{}, shadersDirectory{};
        std::map<std::filesystem::path, std::shared_ptr<Texture>> textures{};
        std::map<std::filesystem::path, std::shared_ptr<ModelData>> modelData{};
        std::map<std::filesystem::path, ShaderFileInfo> shaderFileInfos{};
        std::map<ShaderType, std::shared_ptr<GLShader>> shaders{};
        std::shared_ptr<UboMvp> uboMvp{};
        std::shared_ptr<UboMaterial> uboMaterial{};
        std::shared_ptr<UboLights> uboLights{};
        GLuint quadVao{}, quadVbo{};
    };
}
