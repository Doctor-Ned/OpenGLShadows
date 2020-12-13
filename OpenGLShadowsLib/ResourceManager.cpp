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

void shadow::ResourceManager::updateShaders() const
{
    for (const std::map<ShaderType, std::shared_ptr<GLShader>>::value_type& pair : shaders)
    {
        pair.second->update();
    }
}

std::shared_ptr<shadow::Texture> shadow::ResourceManager::getTexture(const std::filesystem::path& path)
{
    return getTexture(path, true);
}

std::shared_ptr<shadow::ModelMesh> shadow::ResourceManager::getModel(const std::filesystem::path& path)
{
    assert(initialised);
    std::shared_ptr<ModelData> data = getModelData(path);
    if (!data)
    {
        SHADOW_ERROR("Unable to get ModelData from '{}'!", path.generic_string());
        return {};
    }
    std::shared_ptr<ModelMesh> model = std::shared_ptr<ModelMesh>(new ModelMesh(data));
    if (!model->load())
    {
        return {};
    }
    return model;
}

std::shared_ptr<shadow::MaterialModelMesh> shadow::ResourceManager::getMaterialModel(const std::filesystem::path& path, std::shared_ptr<Material> material)
{
    assert(initialised);
    std::shared_ptr<ModelData> data = getModelData(path);
    if (!data)
    {
        SHADOW_ERROR("Unable to get ModelData from '{}'!", path.generic_string());
        return {};
    }
    std::shared_ptr<MaterialModelMesh> model = std::shared_ptr<MaterialModelMesh>(new MaterialModelMesh(data, material));
    if (!model->load())
    {
        return {};
    }
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

std::shared_ptr<shadow::UboMvp> shadow::ResourceManager::getUboMvp() const
{
    return uboMvp;
}

std::shared_ptr<shadow::UboMaterial> shadow::ResourceManager::getUboMaterial() const
{
    return uboMaterial;
}

std::shared_ptr<shadow::Texture> shadow::ResourceManager::getTexture(const std::filesystem::path& path, bool shouldReworkPath)
{
    assert(initialised);
    std::filesystem::path fullPath = shouldReworkPath ? reworkPath(resourceDirectory, MODELS_TEXTURES_DIR, path) : path;
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

std::shared_ptr<shadow::ModelData> shadow::ResourceManager::getModelData(const std::filesystem::path& path)
{
    std::filesystem::path fullPath = reworkPath(resourceDirectory, MODELS_TEXTURES_DIR, path);
    if (!exists(fullPath))
    {
        SHADOW_ERROR("Model file '{}' does not exist!", fullPath.generic_string());
        return {};
    }
    std::map<std::filesystem::path, std::shared_ptr<ModelData>>::iterator it = modelData.find(fullPath);
    if (it != modelData.end())
    {
        return it->second;
    }
    SHADOW_DEBUG("Loading model data from '{}'...", fullPath.generic_string());
    Assimp::Importer import;
    const aiScene* scene = import.ReadFile(fullPath.generic_string().c_str(), aiProcess_Triangulate | aiProcess_FlipUVs);
    if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
    {
        SHADOW_ERROR("Failed to load model '{}'! {}", fullPath.generic_string(), import.GetErrorString());
        return {};
    }
    std::vector<ModelMeshData> modelMeshData{};
    processModelNode(scene->mRootNode, scene, fullPath, modelMeshData);
    std::shared_ptr<ModelData> result = std::make_shared<ModelData>(modelMeshData);
    modelData.emplace(fullPath, result);
    return result;
}

void shadow::ResourceManager::processModelNode(aiNode* node, const aiScene* scene, const std::filesystem::path& path, std::vector<ModelMeshData>& modelMeshData)
{
    for (unsigned int i = 0; i < node->mNumMeshes; ++i)
    {
        aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
        modelMeshData.push_back(processModelMesh(mesh, scene, path));
    }
    for (unsigned int i = 0; i < node->mNumChildren; ++i)
    {
        processModelNode(node->mChildren[i], scene, path, modelMeshData);
    }
}

shadow::ModelMeshData shadow::ResourceManager::processModelMesh(aiMesh* mesh, const aiScene* scene, const std::filesystem::path& path)
{
    ModelMeshData result{};

    for (unsigned int i = 0; i < mesh->mNumVertices; i++)
    {
        result.vertices.emplace_back(mesh->mVertices[i].x, mesh->mVertices[i].y, mesh->mVertices[i].z);
        result.normals.emplace_back(mesh->mNormals[i].x, mesh->mNormals[i].y, mesh->mNormals[i].z);
        if (mesh->mTextureCoords[0])
        {
            result.texCoords.emplace_back(mesh->mTextureCoords[0][i].x, mesh->mTextureCoords[0][i].y);
        } else
        {
            result.texCoords.emplace_back(0.0f, 0.0f);
        }
    }

    for (unsigned int i = 0; i < mesh->mNumFaces; ++i)
    {
        aiFace& face = mesh->mFaces[i];
        for (unsigned int j = 0; j < face.mNumIndices; ++j)
        {
            result.indices.push_back(face.mIndices[j]);
        }
    }

    for (TextureType type : {TextureType::Albedo, TextureType::Roughness, TextureType::Metalness, TextureType::Normal})
    {
        std::shared_ptr<Texture> texture = loadModelTexture(type, path);
        if (texture)
        {
            result.textures.emplace(type, texture);
        }
    }

    return result;
}

std::shared_ptr<shadow::Texture> shadow::ResourceManager::loadModelTexture(TextureType textureType, const std::filesystem::path& path)
{
    std::filesystem::directory_iterator itEnd{};
    for (std::filesystem::directory_iterator it{ path.parent_path() }; it != itEnd; ++it)
    {
        if (is_regular_file(*it))
        {
            std::filesystem::path p = it->path();
            std::string fileName = p.filename().generic_string();
            size_t lastUnderscore = fileName.find_last_of('_'), lastDot = fileName.find_last_of('.');
            //TODO: might add a filetype check
            if (lastUnderscore != std::string::npos && lastDot != std::string::npos)
            {
                ++lastUnderscore;
                std::string type = fileName.substr(lastUnderscore, lastDot - lastUnderscore);
                std::transform(type.begin(), type.end(), type.begin(),
                               [](auto c) { return std::tolower(c); });
                bool match;
                switch (textureType)
                {
                    case TextureType::Albedo:
                        match = !strcmp(type.c_str(), "basecolor") || !strcmp(type.c_str(), "albedo");
                        break;
                    case TextureType::Metalness:
                        match = !strcmp(type.c_str(), "metallic") || !strcmp(type.c_str(), "metalness");
                        break;
                    case TextureType::Roughness:
                        match = !strcmp(type.c_str(), "roughness");
                        break;
                    case TextureType::Normal:
                        match = !strcmp(type.c_str(), "normal");
                        break;
                    default:
                        match = false;
                        SHADOW_ERROR("Encountered unknown texture type {}!", textureType);
                        break;
                }
                if (match)
                {
                    std::shared_ptr<Texture> texture = getTexture(p, false);
                    if (texture)
                    {
                        return texture;
                    }
                }
            }
        }
    }
    return {};
}

void shadow::ResourceManager::loadShaders()
{
    SHADOW_DEBUG("Creating UBOs...");
    uboMvp = std::make_shared<UboMvp>();
    uboMaterial = std::make_shared<UboMaterial>();

    SHADOW_DEBUG("Loading shaders...");
    const std::filesystem::path shadersDirectory = resourceDirectory / SHADERS_DIR;

    shaders.emplace(ShaderType::Texture, std::shared_ptr<GLShader>(new GLShader(shadersDirectory, "Texture")));
    shaders.emplace(ShaderType::Material, std::shared_ptr<GLShader>(new GLShader(shadersDirectory, "Material")));

    for (unsigned int i = 0U; i != static_cast<unsigned int>(ShaderType::ShaderTypeEnd); ++i)
    {
        const std::map<ShaderType, std::shared_ptr<GLShader>>::iterator it = shaders.find(static_cast<ShaderType>(i));
        if (it != shaders.end())
        {
            if (!it->second->createProgram())
            {
                SHADOW_ERROR("Failed to create shader of type {}!", it->first);
            } else
            {
                if (uboMvp->isDeclaredIn(it->second))
                {
                    SHADOW_DEBUG("Binding UBO '{}' to shader of type {}...", uboMvp->getBlockName().cbegin(), i);
                    if (!uboMvp->bindTo(it->second))
                    {
                        SHADOW_ERROR("Failed to bind UBO '{}' to shader of type {}!", uboMvp->getBlockName().cbegin(), i);
                    }
                }
                if (uboMaterial->isDeclaredIn(it->second))
                {
                    SHADOW_DEBUG("Binding UBO '{}' to shader of type {}...", uboMaterial->getBlockName().cbegin(), i);
                    if (!uboMaterial->bindTo(it->second))
                    {
                        SHADOW_ERROR("Failed to bind UBO '{}' to shader of type {}!", uboMaterial->getBlockName().cbegin(), i);
                    }
                }
            }
        }
    }
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
