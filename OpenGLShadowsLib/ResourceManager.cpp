#include "ResourceManager.h"
#include "ShadowLog.h"
#include "GLShader.h"

shadow::ResourceManager& shadow::ResourceManager::getInstance()
{
    static ResourceManager resourceManager{};
    return resourceManager;
}

bool shadow::ResourceManager::initialize(std::filesystem::path resourceDirectory)
{
    assert(!initialised);
    if (!exists(resourceDirectory))
    {
        SHADOW_CRITICAL("Resource directory '{}' was not found!", resourceDirectory.generic_string());
        return false;
    }
    if (!is_directory(resourceDirectory))
    {
        SHADOW_CRITICAL("Path '{}' is not a valid resource directory!", resourceDirectory.generic_string());
        return false;
    }
    if (std::filesystem::path modelsTexturesPath = resourceDirectory / MODELS_TEXTURES_DIR; !exists(modelsTexturesPath))
    {
        SHADOW_CRITICAL("Directory '{}' not found!", modelsTexturesPath.generic_string());
        return false;
    }
    if (std::filesystem::path shadersPath = resourceDirectory / SHADERS_DIR; !exists(shadersPath))
    {
        SHADOW_CRITICAL("Directory '{}' not found!", shadersPath.generic_string());
        return false;
    }
    this->resourceDirectory = resourceDirectory;
    initialised = true;
    loadShaders();
    return true;
}

std::shared_ptr<shadow::Texture> shadow::ResourceManager::getTexture(const std::filesystem::path& path)
{
    assert(initialised);
    std::filesystem::path fullPath = reworkPath(resourceDirectory, MODELS_TEXTURES_DIR, path);
    if (!exists(fullPath))
    {
        SHADOW_ERROR("Texture file '{}' does not exist!", fullPath.generic_string());
        return {};
    }
    std::map<std::filesystem::path, std::shared_ptr<Texture>>::iterator it = textures.find(fullPath);
    if (it != textures.end())
    {
        return it->second;
    }
    std::shared_ptr<Texture> texture = std::shared_ptr<Texture>(new Texture(fullPath));
    if (!texture->load())
    {
        return {};
    }
    textures.emplace(fullPath, texture);
    return texture;
}

std::shared_ptr<shadow::ModelMesh> shadow::ResourceManager::getModel(const std::filesystem::path& path)
{
    assert(initialised);
    std::filesystem::path fullPath = reworkPath(resourceDirectory, MODELS_TEXTURES_DIR, path);
    if (!exists(fullPath))
    {
        SHADOW_ERROR("Model file '{}' does not exist!", fullPath.generic_string());
        return {};
    }
    std::map<std::filesystem::path, std::shared_ptr<ModelMesh>>::iterator it = models.find(fullPath);
    if (it != models.end())
    {
        return it->second;
    }
    std::shared_ptr<ModelMesh> model = std::shared_ptr<ModelMesh>(new ModelMesh(fullPath));
    if (!model->load())
    {
        return {};
    }
    models.emplace(fullPath, model);
    return model;
}

std::shared_ptr<shadow::GLShader> shadow::ResourceManager::getShader(ShaderType shaderType)
{
    if (shaderType == ShaderType::None)
    {
        SHADOW_WARN("An empty shader was requested! This is probably a bug.");
        return {};
    }
    const std::map<ShaderType, std::shared_ptr<GLShader>>::iterator it = shaders.find(shaderType);
    if (it == shaders.end())
    {
        SHADOW_ERROR("Shader of type {} was not found!", shaderType);
        return {};
    }
    return it->second;
}

void shadow::ResourceManager::loadShaders()
{
    SHADOW_DEBUG("Loading shaders...");
    shaders.emplace(ShaderType::None, std::shared_ptr<GLShader>{});
    //todo
}

std::filesystem::path shadow::ResourceManager::reworkPath(const std::filesystem::path& basePath, const std::filesystem::path& midPath, const std::filesystem::path& inputPath)
{
    bool isFull = std::search(basePath.begin(), basePath.end(), inputPath.begin(), inputPath.end()) != basePath.end();
    bool isMidDir = false;
    if (!isFull)
    {
        isMidDir = std::search(midPath.begin(), midPath.end(), inputPath.begin(), inputPath.end()) != midPath.end();
    }
    return
        isFull
        ? inputPath
        : (isMidDir
           ? (basePath / inputPath)
           : (basePath / midPath / inputPath)
           );
}
