#pragma once

#include "Texture.h"
#include "ModelMesh.h"
#include "UboModelViewProjection.h"

#include <memory>
#include <filesystem>
#include <map>

namespace shadow
{
    class ResourceManager
    {
    public:
        ~ResourceManager() = default;
        ResourceManager(ResourceManager&) = delete;
        ResourceManager(ResourceManager&&) = delete;
        ResourceManager& operator=(ResourceManager&) = delete;
        ResourceManager& operator=(ResourceManager&&) = delete;
        static ResourceManager& getInstance();
        bool initialize(std::filesystem::path resourceDirectory);
        void updateShaders() const;
        std::shared_ptr<Texture> getTexture(const std::filesystem::path& path);
        std::shared_ptr<ModelMesh> getModel(const std::filesystem::path& path);
        std::shared_ptr<GLShader> getShader(ShaderType shaderType);
        std::shared_ptr<UboModelViewProjection> getUboMvp() const;
    private:
        ResourceManager() = default;
        void loadShaders();
        bool initialised = false;
        std::filesystem::path resourceDirectory{};
        static std::filesystem::path reworkPath(const std::filesystem::path& basePath, const std::filesystem::path& midPath, const std::filesystem::path& inputPath);
        const std::filesystem::path MODELS_TEXTURES_DIR{ "ModelsTextures" }, SHADERS_DIR{ "Shaders" };
        std::map<std::filesystem::path, std::shared_ptr<Texture>> textures{};
        std::map<std::filesystem::path, std::shared_ptr<ModelMesh>> models{};
        std::map<ShaderType, std::shared_ptr<GLShader>> shaders{};
        std::shared_ptr<UboModelViewProjection> uboMvp{};
    };
}
