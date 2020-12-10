#pragma once

#include "Texture.h"
#include "ModelMesh.h"

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
        std::shared_ptr<Texture> getTexture(const std::filesystem::path& path);
        std::shared_ptr<ModelMesh> getModel(const std::filesystem::path& path);
    private:
        ResourceManager() = default;
        std::map<std::filesystem::path, std::shared_ptr<Texture>> textures{};
        std::map<std::filesystem::path, std::shared_ptr<ModelMesh>> models{};
    };
}
