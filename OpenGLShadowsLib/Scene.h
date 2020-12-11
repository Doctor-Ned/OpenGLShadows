#pragma once

#include "SceneNode.h"

#include <memory>

namespace shadow
{
    class Scene final
    {
    public:
        Scene() = default;
        ~Scene() = default;
        Scene(Scene&) = delete;
        Scene(Scene&&) = delete;
        Scene& operator=(Scene&) = delete;
        Scene& operator=(Scene&&) = delete;
        std::shared_ptr<SceneNode> getRoot() const;
        std::shared_ptr<SceneNode> addNode(std::shared_ptr<SceneNode> parent) const;
        std::shared_ptr<SceneNode> addNode() const;
        bool removeNode(std::shared_ptr<SceneNode> node) const;
        bool setParent(std::shared_ptr<SceneNode> parent, std::shared_ptr<SceneNode> child) const;
    private:
        static bool isInTree(std::shared_ptr<SceneNode> tree, std::shared_ptr<SceneNode> node);
        std::shared_ptr<SceneNode> root{};
    };
}
