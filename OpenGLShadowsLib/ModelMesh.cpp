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
    std::map<TextureType, std::vector<std::shared_ptr<Texture>>> textures;

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

    if (mesh->mMaterialIndex >= 0)
    {
        aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];
        std::vector<std::shared_ptr<Texture>>
            albedo = loadTextures(material, aiTextureType_BASE_COLOR),
            roughness = loadTextures(material, aiTextureType_DIFFUSE_ROUGHNESS),
            metalness = loadTextures(material, aiTextureType_METALNESS),
            normal = loadTextures(material, aiTextureType_NORMAL_CAMERA);
        textures.emplace(TextureType::Albedo, albedo);
        textures.emplace(TextureType::Roughness, roughness);
        textures.emplace(TextureType::Metalness, metalness);
        textures.emplace(TextureType::Normal, normal);
    }

    return std::make_shared<TextureMesh>(vertices, indices, textures);
}

std::vector<std::shared_ptr<shadow::Texture>> shadow::ModelMesh::loadTextures(aiMaterial* mat, aiTextureType type) const
{
    std::vector<std::shared_ptr<Texture>> result{};
    for (unsigned int i = 0; i < mat->GetTextureCount(type); ++i)
    {
        aiString str;
        mat->GetTexture(type, 0, &str);
        
        std::shared_ptr<Texture> texture = ResourceManager::getInstance().getTexture(path.parent_path().append(str.C_Str()));
        if (texture)
        {
            result.push_back(texture);
        }
    }
    return result;
}
