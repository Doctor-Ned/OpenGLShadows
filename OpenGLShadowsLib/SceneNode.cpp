#include "SceneNode.h"
#include "Scene.h"

#include <glm/gtx/transform.hpp>

bool shadow::SceneNode::isActive()
{
    if (activeDirty)
    {
        std::shared_ptr<SceneNode> p = parent.lock();
        if (p)
        {
            active = p->isActive() && activeSelf;
        } else
        {
            active = activeSelf;
        }
        activeDirty = false;
    }
    return active;
}

bool shadow::SceneNode::isActiveSelf() const
{
    return activeSelf;
}

glm::mat4 shadow::SceneNode::getModel() const
{
    return model;
}

glm::mat4 shadow::SceneNode::getWorld()
{
    if (dirty)
    {
        std::shared_ptr<SceneNode> p = parent.lock();
        if (p)
        {
            world = p->getWorld() * model;
        } else
        {
            world = model;
        }
        dirty = false;
    }
    return world;
}

std::shared_ptr<shadow::Mesh> shadow::SceneNode::getMesh() const
{
    return mesh;
}

shadow::SceneNode& shadow::SceneNode::setActiveSelf(bool activeSelf)
{
    this->activeSelf = activeSelf;
    return *this;
}

shadow::SceneNode& shadow::SceneNode::setModel(glm::mat4 model)
{
    this->model = model;
    return *this;
}

shadow::SceneNode& shadow::SceneNode::setMesh(std::shared_ptr<Mesh> mesh)
{
    this->mesh = mesh;
    return *this;
}

shadow::SceneNode& shadow::SceneNode::translate(glm::vec3 vec)
{
    model = glm::translate(model, vec);
    dirty = true;
    return *this;
}

shadow::SceneNode& shadow::SceneNode::scale(glm::vec3 vec)
{
    model = glm::scale(model, vec);
    dirty = true;
    return *this;
}

shadow::SceneNode& shadow::SceneNode::rotate(float angle, glm::vec3 axis)
{
    model = glm::rotate(angle, axis);
    dirty = true;
    return *this;
}

std::shared_ptr<shadow::SceneNode> shadow::SceneNode::getParent() const
{
    return parent.lock();
}

std::vector<std::shared_ptr<shadow::SceneNode>> shadow::SceneNode::getChildren() const
{
    return children;
}

void shadow::SceneNode::setDirty()
{
    dirty = true;
    for (const std::shared_ptr<SceneNode>& node : children)
    {
        node->setDirty();
    }
}

void shadow::SceneNode::setActiveDirty()
{
    activeDirty = true;
    for (const std::shared_ptr<SceneNode>& node : children)
    {
        node->setActiveDirty();
    }
}
