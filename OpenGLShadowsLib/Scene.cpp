#include "Scene.h"
#include <vector>

std::shared_ptr<shadow::SceneNode> shadow::Scene::getRoot() const
{
    return root;
}

std::shared_ptr<shadow::SceneNode> shadow::Scene::addNode(std::shared_ptr<SceneNode> parent) const
{
    if (!parent)
    {
        parent = root;
    } else
    {
        assert(isInTree(root, parent));
    }
    std::shared_ptr<SceneNode> node{ new SceneNode() };
    node->parent = parent;
    parent->children.push_back(node);
    return node;
}

std::shared_ptr<shadow::SceneNode> shadow::Scene::addNode() const
{
    return addNode(root);
}

bool shadow::Scene::removeNode(std::shared_ptr<SceneNode> node) const
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
