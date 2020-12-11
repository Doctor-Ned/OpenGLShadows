#include "MaterialModelMesh.h"

#include <utility>
#include "ShadowLog.h"
#include "ResourceManager.h"

shadow::MaterialModelMesh::MaterialModelMesh(std::filesystem::path path, std::shared_ptr<Material> material)
    : material(std::move(material)), path(std::move(path))
{}

bool shadow::MaterialModelMesh::load()
{
    if (!meshes.empty())
    {
        return true;
    }
    if (!material)
    {
        SHADOW_ERROR("Unable to proceed - provided null material!");
        return false;
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

std::filesystem::path shadow::MaterialModelMesh::getPath() const
{
    return path;
}

void shadow::MaterialModelMesh::draw(std::shared_ptr<GLShader> shader, glm::mat4 model) const
{
    assert(!meshes.empty());
    for (const std::shared_ptr<MaterialMesh>& mesh : meshes)
    {
        mesh->draw(shader, model);
    }
}

shadow::ShaderType shadow::MaterialModelMesh::getShaderType() const
{
    return ShaderType::Material;
}

std::shared_ptr<shadow::Material> shadow::MaterialModelMesh::getMaterial() const
{
    return material;
}

void shadow::MaterialModelMesh::setMaterial(std::shared_ptr<Material> material)
{
    assert(material);
    this->material = material;
    for (const std::shared_ptr<MaterialMesh>& mesh : meshes)
    {
        mesh->setMaterial(material);
    }
}

void shadow::MaterialModelMesh::processNode(aiNode* node, const aiScene* scene)
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

std::shared_ptr<shadow::MaterialMesh> shadow::MaterialModelMesh::processMesh(aiMesh* mesh, const aiScene* scene) const
{
    std::vector<Vertex> vertices;
    std::vector<GLuint> indices;

    for (unsigned int i = 0; i < mesh->mNumVertices; i++)
    {
        Vertex vertex{
            {mesh->mVertices[i].x,mesh->mVertices[i].y,mesh->mVertices[i].z},
            {mesh->mNormals[i].x,mesh->mNormals[i].y,mesh->mNormals[i].z}
        };
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

    return std::make_shared<MaterialMesh>(vertices, indices, material);
}
