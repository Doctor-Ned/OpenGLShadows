#include "ModelMesh.h"
#include "ShadowLog.h"
#include "ResourceManager.h"

shadow::ModelMesh::ModelMesh(std::shared_ptr<ModelData> modelData)
    : modelData(modelData)
{}

bool shadow::ModelMesh::load()
{
    if (!meshes.empty())
    {
        return true;
    }
    if (!modelData)
    {
        SHADOW_ERROR("Unable to proceed - provided null modelData!");
        return false;
    }
    for (const ModelMeshData& meshData : modelData->modelMeshData)
    {
        std::vector<TextureVertex> vertices{};
        for (unsigned int i = 0; i < meshData.vertices.size(); ++i)
        {
            vertices.push_back(TextureVertex{ meshData.vertices[i], meshData.normals[i], meshData.texCoords[i] });
        }
        meshes.push_back(std::make_shared<TextureMesh>(vertices, meshData.indices, meshData.textures));
    }
    return true;
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
