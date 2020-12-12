#include "ModelMesh.h"
#include "ShadowLog.h"
#include "ResourceManager.h"

shadow::ModelMesh::ModelMesh(std::filesystem::path path) : path(std::move(path)) {}

bool shadow::ModelMesh::load()
{
    if (!meshes.empty())
    {
        return true;
    }
    SHADOW_DEBUG("Loading model '{}'...", path.generic_string());
    if (!exists(path))
    {
        SHADOW_ERROR("Model file '{}' does not exist!", path.generic_string());
        return false;
    }

    Assimp::Importer import;
    const aiScene* scene = import.ReadFile(path.generic_string().c_str(), aiProcess_Triangulate | aiProcess_FlipUVs);
    if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
    {
        SHADOW_ERROR("Failed to load model '{}'! {}", path.generic_string(), import.GetErrorString());
        return false;
    }
    processNode(scene->mRootNode, scene);
    if (meshes.empty())
    {
        SHADOW_ERROR("Assimp loaded the model '{}', but no meshes were created!", path.generic_string());
        return false;
    }
    return true;
}

std::filesystem::path shadow::ModelMesh::getPath() const
{
    return path;
}

void shadow::ModelMesh::draw(std::shared_ptr<GLShader> shader) const
{
    assert(!meshes.empty());
    for (const std::shared_ptr<TextureMesh>& mesh : meshes)
    {
        mesh->draw(shader);
    }
}

shadow::ShaderType shadow::ModelMesh::getShaderType() const
{
    return ShaderType::Texture;
}

void shadow::ModelMesh::processNode(aiNode* node, const aiScene* scene)
{
    for (unsigned int i = 0; i < node->mNumMeshes; ++i)
    {
        aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
        meshes.push_back(processMesh(mesh, scene));
    }
    for (unsigned int i = 0; i < node->mNumChildren; ++i)
    {
        processNode(node->mChildren[i], scene);
    }
}

std::shared_ptr<shadow::TextureMesh> shadow::ModelMesh::processMesh(aiMesh* mesh, const aiScene* scene) const
{
    std::vector<TextureVertex> vertices;
    std::vector<GLuint> indices;
    std::map<TextureType, std::shared_ptr<Texture>> textures;

    for (unsigned int i = 0; i < mesh->mNumVertices; i++)
    {
        TextureVertex vertex{
            {mesh->mVertices[i].x,mesh->mVertices[i].y,mesh->mVertices[i].z},
            {mesh->mNormals[i].x,mesh->mNormals[i].y,mesh->mNormals[i].z},
            {}
        };
        if (mesh->mTextureCoords[0])
        {
            vertex.texCoords = { mesh->mTextureCoords[0][i].x, mesh->mTextureCoords[0][i].y };
        }
        vertices.push_back(vertex);
    }

    for (unsigned int i = 0; i < mesh->mNumFaces; ++i)
    {
        aiFace& face = mesh->mFaces[i];
        for (unsigned int j = 0; j < face.mNumIndices; ++j)
        {
            indices.push_back(face.mIndices[j]);
        }
    }

    for (TextureType type : {TextureType::Albedo, TextureType::Roughness, TextureType::Metalness, TextureType::Normal})
    {
        std::shared_ptr<Texture> texture = loadTexture(type);
        if (texture)
        {
            textures.emplace(type, texture);
        }
    }

    return std::make_shared<TextureMesh>(vertices, indices, textures);
}

std::shared_ptr<shadow::Texture> shadow::ModelMesh::loadTexture(TextureType textureType) const
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
                    std::shared_ptr<Texture> texture = ResourceManager::getInstance().getTexture(p);
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
