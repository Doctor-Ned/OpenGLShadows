#include "ResourceManager.h"
#include "ShadowLog.h"
#include "GLShader.h"
#include "ShadowUtils.h"
#include "Vertex2D.h"

#include <fstream>
#include <streambuf>
#include <sstream>

shadow::ResourceManager::~ResourceManager()
{
    glDeleteBuffers(1, &quadVbo);
    glDeleteVertexArrays(1, &quadVao);
}

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
    modelsTexturesDirectory = resourceDirectory / MODELS_TEXTURES_DIR;
    if (!exists(modelsTexturesDirectory) || !is_directory(modelsTexturesDirectory))
    {
        SHADOW_CRITICAL("Directory '{}' not found!", modelsTexturesDirectory.generic_string());
        return false;
    }
    shadersDirectory = resourceDirectory / SHADERS_DIR;
    if (!exists(shadersDirectory) || !is_directory(shadersDirectory))
    {
        SHADOW_CRITICAL("Directory '{}' not found!", shadersDirectory.generic_string());
        return false;
    }

    Vertex2D screenQuadVertices[6]
    {
        {{-1.0f, -1.0f},
        {0.0f, 0.0f}},
        {{1.0f, 1.0f},
        {1.0f, 1.0f}},
        {{-1.0f, 1.0f},
        {0.0f, 1.0f}},
        {{-1.0f, -1.0f},
        {0.0f, 0.0f}},
        {{1.0f, -1.0f},
        {1.0f, 0.0f}},
        {{1.0f, 1.0f},
        {1.0f, 1.0f}}
    };
    glGenVertexArrays(1, &quadVao);
    glGenBuffers(1, &quadVbo);
    glBindVertexArray(quadVao);
    glBindBuffer(GL_ARRAY_BUFFER, quadVbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(Vertex2D) * 6, screenQuadVertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex2D), reinterpret_cast<void*>(offsetof(Vertex2D, position)));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex2D), reinterpret_cast<void*>(offsetof(Vertex2D, texCoords)));
    glBindVertexArray(0);
    this->resourceDirectory = resourceDirectory;
    initialised = true;
    loadShaders();
    return true;
}

void shadow::ResourceManager::reworkShaderFiles()
{
    // check for files that were modified
    for (const std::filesystem::directory_entry& file : std::filesystem::directory_iterator(shadersDirectory))
    {
        if (is_regular_file(file))
        {
            const std::filesystem::path& path = file.path();
            std::map<std::filesystem::path, ShaderFileInfo>::iterator it = shaderFileInfos.find(path);
            if (it != shaderFileInfos.end())
            {
                std::filesystem::file_time_type timestamp = last_write_time(path);
                if (it->second.timestamp != timestamp)
                {
                    it->second.timestamp = timestamp;
                    it->second.content.clear();
                    it->second.references.clear();
                    it->second.modified = true;
                }
            } else
            {
                shaderFileInfos.emplace(path, ShaderFileInfo{ {}, last_write_time(path), {}, true });
            }
        }
    }
    // gather references for modified files
    for (std::map<std::filesystem::path, ShaderFileInfo>::value_type& pair : shaderFileInfos)
    {
        if (pair.second.modified)
        {
            std::ifstream stream(pair.first);
            std::string line;
            while (std::getline(stream, line))
            {
                std::string includedFile{};
                if (line.rfind(INCLUDE_TEXT, 0) == 0)
                {
                    includedFile = line.substr(INCLUDE_LENGTH);
                } else if (line.rfind(INCLUDED_FROM_TEXT, 0) == 0)
                {
                    includedFile = line.substr(INCLUDED_FROM_LENGTH);
                }
                if (!includedFile.empty())
                {
                    bool fileFound = false;
                    for (std::map<std::filesystem::path, ShaderFileInfo>::value_type& innerPair : shaderFileInfos)
                    {
                        if (innerPair.first.filename() == includedFile)
                        {
                            fileFound = true;
                            if (isShaderFileRecursivelyReferenced(pair.first, innerPair.first))
                            {
                                SHADOW_WARN("Recursive reference of '{}' found in '{}'! This reference will NOT be included.",
                                            pair.first.generic_string(), innerPair.first.generic_string());
                            } else
                            {
                                if (std::find(pair.second.references.begin(), pair.second.references.end(), innerPair.first) == pair.second.references.end())
                                {
                                    pair.second.references.push_back(innerPair.first);
                                }
                            }
                            break;
                        }
                    }
                    if (!fileFound)
                    {
                        SHADOW_ERROR("File '{}' referenced in '{}' was NOT found!", includedFile, pair.first.generic_string());
                    }
                }
            }
        }
    }
    // rework modified status basing on references
    for (std::map<std::filesystem::path, ShaderFileInfo>::value_type& pair : shaderFileInfos)
    {
        if (!pair.second.modified)
        {
            pair.second.modified = isShaderFileModified(pair.first);
        }
    }
    // rebuild files if needed
    for (std::map<std::filesystem::path, ShaderFileInfo>::value_type& pair : shaderFileInfos)
    {
        if (!pair.second.modified && !rebuildShaderFile(pair.first))
        {
            SHADOW_ERROR("Failed to rebuild shader file '{}'!", pair.first.generic_string());
        }
    }
    // clear modified status
    for (std::map<std::filesystem::path, ShaderFileInfo>::value_type& pair : shaderFileInfos)
    {
        pair.second.modified = false;
    }
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

std::shared_ptr<shadow::UboLights> shadow::ResourceManager::getUboLights() const
{
    return uboLights;
}

void shadow::ResourceManager::renderQuad() const
{
    glBindVertexArray(quadVao);
    glBindVertexBuffer(0, quadVbo, 0, sizeof(Vertex2D));
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindVertexArray(0);
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
    const aiScene* scene = import.ReadFile(fullPath.generic_string().c_str(), aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_GenSmoothNormals | aiProcess_CalcTangentSpace);
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
        result.tangents.emplace_back(mesh->mTangents[i].x, mesh->mTangents[i].y, mesh->mTangents[i].z);
        result.bitangents.emplace_back(mesh->mBitangents[i].x, mesh->mBitangents[i].y, mesh->mBitangents[i].z);
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
                        match = !strcmp(type.c_str(), "bc") || !strcmp(type.c_str(), "basecolor") || !strcmp(type.c_str(), "albedo");
                        break;
                    case TextureType::Metalness:
                        match = !strcmp(type.c_str(), "m") || !strcmp(type.c_str(), "metallic") || !strcmp(type.c_str(), "metalness");
                        break;
                    case TextureType::Roughness:
                        match = !strcmp(type.c_str(), "r") || !strcmp(type.c_str(), "roughness");
                        break;
                    case TextureType::Normal:
                        match = !strcmp(type.c_str(), "n") || !strcmp(type.c_str(), "normal");
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

bool shadow::ResourceManager::rebuildShaderFile(const std::filesystem::path& path)
{
    const std::map<std::filesystem::path, ShaderFileInfo>::iterator it = shaderFileInfos.find(path);
    if (it == shaderFileInfos.end())
    {
        SHADOW_ERROR("File '{}' was not found in shaderFileInfos! This should NOT happen.", path.generic_string());
        return false;
    }
    if (!it->second.modified || !it->second.content.empty())
    {
        return true;
    }
    std::ifstream stream(it->first);
    if (it->second.references.empty())
    {
        it->second.content = std::string(std::istreambuf_iterator<char>(stream), std::istreambuf_iterator<char>());
    } else
    {
        for (const std::filesystem::path& refPath : it->second.references)
        {
            if (!rebuildShaderFile(refPath))
            {
                return false;
            }
        }
        std::vector<std::string> lines{};
        std::string line;
        while (std::getline(stream, line))
        {
            lines.push_back(line);
        }
        std::stringstream ss{};
        for (size_t i = 0; i < lines.size();)
        {
            if (lines[i].rfind(INCLUDE_TEXT, 0) == 0)
            {
                std::string includedFile = lines[i].substr(INCLUDE_LENGTH);
                for (const std::map<std::filesystem::path, ShaderFileInfo>::value_type& pair : shaderFileInfos)
                {
                    if (pair.first.filename() == includedFile)
                    {
                        ss << INCLUDED_FROM_TEXT << includedFile << std::endl << pair.second.content << std::endl << END_INCLUDE_TEXT << includedFile << std::endl;
                        break;
                    }
                }
                ++i;
            } else if (lines[i].rfind(INCLUDED_FROM_TEXT, 0) == 0)
            {
                std::string includedFile = lines[i].substr(INCLUDED_FROM_LENGTH);
                size_t endIndex = std::string::npos;
                for (size_t j = i + 1; j < lines.size(); ++j)
                {
                    if (lines[j].rfind(END_INCLUDE_TEXT, 0) == 0)
                    {
                        if (lines[j].substr(END_INCLUDE_LENGTH) == includedFile)
                        {
                            endIndex = j;
                            break;
                        }
                    }
                }
                if (endIndex == std::string::npos)
                {
                    SHADOW_ERROR("Unable to find inclusion end ('{}' included at line {} in '{}')!", includedFile, i + 1, path.generic_string());
                } else
                {
                    lines.erase(lines.begin() + i, lines.begin() + endIndex);
                    bool fileFound = false;
                    for (const std::map<std::filesystem::path, ShaderFileInfo>::value_type& pair : shaderFileInfos)
                    {
                        if (pair.first.filename() == includedFile)
                        {
                            fileFound = true;
                            ss << INCLUDED_FROM_TEXT << includedFile << std::endl << pair.second.content << std::endl << END_INCLUDE_TEXT << includedFile << std::endl;
                            break;
                        }
                    }
                    ++i;
                    if (!fileFound)
                    {
                        ss << INCLUDE_TEXT << includedFile << std::endl;
                    }
                }
            } else
            {
                ss << lines[i] << std::endl;
                ++i;
            }
        }
        it->second.content = ss.str();
    }
    return true;
}

bool shadow::ResourceManager::isShaderFileRecursivelyReferenced(const std::filesystem::path& path, const std::filesystem::path& searchPath)
{
    const std::map<std::filesystem::path, ShaderFileInfo>::iterator it = shaderFileInfos.find(searchPath);
    if (it == shaderFileInfos.end())
    {
        SHADOW_ERROR("File '{}' was not found in shaderFileInfos! This should NOT happen.", searchPath.generic_string());
        return false;
    }
    for (const std::filesystem::path& refPath : it->second.references)
    {
        if (refPath == path)
        {
            return true;
        }
        if (isShaderFileRecursivelyReferenced(path, refPath))
        {
            return true;
        }
    }
    return false;
}

bool shadow::ResourceManager::isShaderFileModified(const std::filesystem::path& path)
{
    const std::map<std::filesystem::path, ShaderFileInfo>::iterator it = shaderFileInfos.find(path);
    if (it == shaderFileInfos.end())
    {
        SHADOW_ERROR("File '{}' was not found in shaderFileInfos! This should NOT happen.", path.generic_string());
        return false;
    }
    if (it->second.modified)
    {
        return true;
    }
    for (const std::filesystem::path& refPath : it->second.references)
    {
        if (isShaderFileModified(refPath))
        {
            return true;
        }
    }
    return false;
}

void shadow::ResourceManager::loadShaders()
{
    SHADOW_DEBUG("Reworking shader files...");
    reworkShaderFiles();
    SHADOW_DEBUG("Loading shaders...");
    shaders.emplace(ShaderType::Texture, std::shared_ptr<GLShader>(new GLShader(shadersDirectory, "Texture")));
    shaders.emplace(ShaderType::Material, std::shared_ptr<GLShader>(new GLShader(shadersDirectory, "Material")));
    shaders.emplace(ShaderType::DepthDir, std::shared_ptr<GLShader>(new GLShader(shadersDirectory, "DepthDir.vert", "Depth.frag")));
    shaders.emplace(ShaderType::DepthSpot, std::shared_ptr<GLShader>(new GLShader(shadersDirectory, "DepthSpot.vert", "Depth.frag")));
    shaders.emplace(ShaderType::DepthDirVSM, std::shared_ptr<GLShader>(new GLShader(shadersDirectory, "DepthDir.vert", "DepthVSM.frag")));
    shaders.emplace(ShaderType::DepthSpotVSM, std::shared_ptr<GLShader>(new GLShader(shadersDirectory, "DepthSpot.vert", "DepthVSM.frag")));
    shaders.emplace(ShaderType::GaussianBlur, std::shared_ptr<GLShader>(new GLShader(shadersDirectory, "PostProcess.vert", "GaussianBlur.frag")));
    shaders.emplace(ShaderType::PostProcess, std::shared_ptr<GLShader>(new GLShader(shadersDirectory, "PostProcess")));
    for (unsigned int i = 0U; i != static_cast<unsigned int>(ShaderType::ShaderTypeEnd); ++i)
    {
        const std::map<ShaderType, std::shared_ptr<GLShader>>::iterator it = shaders.find(static_cast<ShaderType>(i));
        if (it != shaders.end())
        {
            if (!it->second->createProgram())
            {
                SHADOW_ERROR("Failed to create shader of type {}!", it->first);
            }
        }
    }

    SHADOW_DEBUG("Creating UBOs...");
    uboMvp = std::make_shared<UboMvp>();
    uboMaterial = std::make_shared<UboMaterial>();
    DirectionalLightData dirLightData{};
    SpotLightData spotLightData{};
    uboLights = std::make_shared<UboLights>(
        std::make_shared<DirectionalLight>(dirLightData),
        std::make_shared<SpotLight>(spotLightData));
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
