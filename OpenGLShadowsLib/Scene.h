#pragma once

#include "SceneNode.h"
#include <Camera.h>

#include <memory>
#include <map>

namespace shadow
{
    class Scene final
    {
    public:
        Scene(std::shared_ptr<Camera> camera);
        ~Scene() = default;
        Scene(Scene&) = delete;
        Scene(Scene&&) = delete;
        Scene& operator=(Scene&) = delete;
        Scene& operator=(Scene&&) = delete;
        std::shared_ptr<SceneNode> getRoot() const;
        std::shared_ptr<SceneNode> addNode(std::shared_ptr<SceneNode> parent);
        std::shared_ptr<SceneNode> addNode();
        bool removeNode(std::shared_ptr<SceneNode> node);
        bool setParent(std::shared_ptr<SceneNode> parent, std::shared_ptr<SceneNode> child) const;
        void render(std::shared_ptr<GLShader> overrideShader);
        std::shared_ptr<Camera> getCamera() const;
    private:
        friend class SceneNode;
        static bool isInTree(std::shared_ptr<SceneNode> tree, std::shared_ptr<SceneNode> node);
        static void renderWithShader(std::shared_ptr<SceneNode> node, std::shared_ptr<GLShader> shader);
        void updateNodeShaderType(ShaderType previous, std::shared_ptr<SceneNode> node);
        std::shared_ptr<SceneNode> root{};
        std::map<ShaderType, std::vector<std::shared_ptr<SceneNode>>> shaderMap{};
        std::shared_ptr<Camera> camera{};
    };
}
