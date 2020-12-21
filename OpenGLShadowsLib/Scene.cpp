#include "Scene.h"
#include "ResourceManager.h"
#include "GLShader.h"

#include <vector>

bool shadow::Scene::initialize(std::shared_ptr<Camera> camera)
{
    if (!camera)
    {
        SHADOW_ERROR("Cannot initialize scene with null camera!");
        return false;
    }
    this->camera = camera;
    this->uboMvp = ResourceManager::getInstance().getUboMvp();
    if (!uboMvp)
    {
        SHADOW_ERROR("Cannot proceed with uninitialized UboMvp!");
        return false;
    }
    this->uboMaterial = ResourceManager::getInstance().getUboMaterial();
    if (!uboMaterial)
    {
        SHADOW_ERROR("Cannot proceed with uninitialized UboMaterial!");
        return false;
    }
    this->uboLights = ResourceManager::getInstance().getUboLights();
    if (!uboLights)
    {
        SHADOW_ERROR("Cannot proceed with uninitialized UboLights!");
        return false;
    }
    root = std::shared_ptr<SceneNode>(new SceneNode());
    root->scene = shared_from_this();
    for (unsigned int i = 0U; i != static_cast<unsigned int>(ShaderType::ShaderTypeEnd); ++i)
    {
        shaderMap.emplace(static_cast<ShaderType>(i), std::vector<std::shared_ptr<SceneNode>>());
    }
    return true;
}

std::shared_ptr<shadow::SceneNode> shadow::Scene::getRoot() const
{
    return root;
}

std::shared_ptr<shadow::SceneNode> shadow::Scene::addNode(std::shared_ptr<SceneNode> parent)
{
    if (!parent)
    {
        parent = root;
    } else
    {
        assert(isInTree(root, parent));
    }
    std::shared_ptr<SceneNode> node{ new SceneNode() };
    shaderMap[ShaderType::None].push_back(node);
    node->parent = parent;
    node->scene = shared_from_this();
    parent->children.push_back(node);
    return node;
}

std::shared_ptr<shadow::SceneNode> shadow::Scene::addNode()
{
    return addNode(root);
}

bool shadow::Scene::removeNode(std::shared_ptr<SceneNode> node)
{
    assert(node);
    if (root == node)
    {
        return false;
    }
    assert(isInTree(root, node));
    std::shared_ptr<SceneNode> parent = node->getParent();
    assert(parent);
    std::vector<std::shared_ptr<SceneNode>>::iterator it =
        std::find(parent->children.begin(), parent->children.end(), node);
    parent->children.erase(it);
    std::vector<std::shared_ptr<SceneNode>>& shaderVec = shaderMap[node->getMesh() ? node->getMesh()->getShaderType() : ShaderType::None];
    it = std::find(shaderVec.begin(), shaderVec.end(), node);
    assert(it != shaderVec.end());
    shaderVec.erase(it);
    return true;
}

bool shadow::Scene::setParent(std::shared_ptr<SceneNode> parent, std::shared_ptr<SceneNode> child) const
{
    assert(child);
    if (!parent)
    {
        parent = root;
    } else if (isInTree(child, parent))
    {
        SHADOW_WARN("Attempted to change child of a node creating a reference loop. Changes were prevented.");
        return false;
    }
    std::shared_ptr<SceneNode> currParent = child->getParent();
    if (parent != currParent)
    {
        std::vector<std::shared_ptr<SceneNode>>::iterator it =
            std::find(currParent->children.begin(), currParent->children.end(), child);
        currParent->children.erase(it);
        parent->children.push_back(child);
        child->parent = parent;
        child->setDirty();
    }
    return true;
}

void shadow::Scene::render()
{
    render({});
}

void shadow::Scene::render(std::shared_ptr<GLShader> overrideShader)
{
    static ResourceManager& resourceManager = ResourceManager::getInstance();
    if (overrideShader)
    {
        renderWithShader(root, overrideShader);
    } else
    {
        for (std::map<ShaderType, std::vector<std::shared_ptr<SceneNode>>>::value_type& pair : shaderMap)
        {
            if (pair.first != ShaderType::None && !pair.second.empty())
            {
                std::shared_ptr<GLShader> shader = resourceManager.getShader(pair.first);
                shader->use();
                for (std::shared_ptr<SceneNode>& node : pair.second)
                {
                    if (node->isActive())
                    {
                        assert(node->getMesh());
                        glm::mat4 model = node->getWorld();
                        uboMvp->setModel(model);
                        node->getMesh()->draw(shader);
                    }
                }
            }
        }
    }
}

std::shared_ptr<shadow::Camera> shadow::Scene::getCamera() const
{
    return camera;
}

bool shadow::Scene::isInTree(std::shared_ptr<SceneNode> tree, std::shared_ptr<SceneNode> node)
{
    if (!node)
    {
        return false;
    }
    assert(tree);
    if (tree == node)
    {
        return true;
    }
    for (const std::shared_ptr<SceneNode>& child : tree->getChildren())
    {
        if (isInTree(child, node))
        {
            return true;
        }
    }
    return false;
}

void shadow::Scene::renderWithShader(std::shared_ptr<SceneNode> node, std::shared_ptr<GLShader> shader) const
{
    assert(shader);
    if (!node->isActive())
    {
        return;
    }
    std::shared_ptr<Mesh> mesh = node->getMesh();
    if (mesh)
    {
        glm::mat4 model = node->getWorld();
        uboMvp->setModel(model);
        mesh->draw(shader);
    }
    for (const std::shared_ptr<SceneNode>& child : node->getChildren())
    {
        renderWithShader(child, shader);
    }
}

void shadow::Scene::updateNodeShaderType(ShaderType previous, std::shared_ptr<SceneNode> node)
{
    ShaderType targetType = node->getMesh() ? node->getMesh()->getShaderType() : ShaderType::None;
    if (targetType == previous)
    {
        return;
    }
    std::vector<std::shared_ptr<SceneNode>>& source = shaderMap[previous];
    std::vector<std::shared_ptr<SceneNode>>& target = shaderMap[targetType];
    const std::vector<std::shared_ptr<SceneNode>>::iterator it =
        std::find(source.begin(), source.end(), node);
    assert(it != source.end());
    source.erase(it);
    target.push_back(node);
}
