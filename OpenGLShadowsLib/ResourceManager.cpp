#include "ResourceManager.h"
#include "ShadowLog.h"

shadow::ResourceManager& shadow::ResourceManager::getInstance()
{
    static ResourceManager resourceManager{};
    return resourceManager;
}

std::shared_ptr<shadow::Texture> shadow::ResourceManager::getTexture(const std::filesystem::path& path)
{
    if (!exists(path))
    {
        SHADOW_ERROR("Texture file '{}' does not exist!", path.generic_string());
        return {};
    }
    std::map<std::filesystem::path, std::shared_ptr<Texture>>::iterator it = textures.find(path);
    if (it != textures.end())
    {
        return it->second;
    }
    std::shared_ptr<Texture> texture = std::shared_ptr<Texture>(new Texture(path));
    if (!texture->load())
    {
        return {};
    }
    textures.emplace(path, texture);
    return texture;
}

std::shared_ptr<shadow::ModelMesh> shadow::ResourceManager::getModel(const std::filesystem::path& path)
{
    if (!exists(path))
    {
        SHADOW_ERROR("Model file '{}' does not exist!", path.generic_string());
        return {};
    }
    std::map<std::filesystem::path, std::shared_ptr<ModelMesh>>::iterator it = models.find(path);
    if (it != models.end())
    {
        return it->second;
    }
    std::shared_ptr<ModelMesh> model = std::shared_ptr<ModelMesh>(new ModelMesh(path));
    if (!model->load())
    {
        return {};
    }
    models.emplace(path, model);
    return model;
}
