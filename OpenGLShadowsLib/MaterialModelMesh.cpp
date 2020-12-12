#include "MaterialModelMesh.h"
#include "ShadowLog.h"
#include "ResourceManager.h"

shadow::MaterialModelMesh::MaterialModelMesh(std::shared_ptr<ModelData> modelData, std::shared_ptr<Material> material)
    : modelData(modelData), material(material)
{}

bool shadow::MaterialModelMesh::load()
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
    if (!material)
    {
        SHADOW_ERROR("Unable to proceed - provided null material!");
        return false;
    }
    for (const ModelMeshData& meshData : modelData->modelMeshData)
    {
        std::vector<Vertex> vertices{};
        for (unsigned int i = 0; i < meshData.vertices.size(); ++i)
        {
            vertices.push_back(Vertex{ meshData.vertices[i], meshData.normals[i] });
        }
        meshes.push_back(std::make_shared<MaterialMesh>(vertices, meshData.indices, material));
    }
    return true;
}

void shadow::MaterialModelMesh::draw(std::shared_ptr<GLShader> shader) const
{
    assert(!meshes.empty());
    for (const std::shared_ptr<MaterialMesh>& mesh : meshes)
    {
        mesh->draw(shader);
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
