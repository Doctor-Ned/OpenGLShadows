#pragma once

#include "Camera.h"
#include "SceneNode.h"
#include "UboMvp.h"
#include "UboMaterial.h"

#include <memory>
#include <map>

namespace shadow
{
    class SceneNode;
    class Scene final : public std::enable_shared_from_this<Scene>
    {
    public:
        Scene() = default;
        ~Scene() = default;
        Scene(Scene&) = delete;
        Scene(Scene&&) = delete;
        Scene& operator=(Scene&) = delete;
        Scene& operator=(Scene&&) = delete;
        bool initialize(std::shared_ptr<Camera> camera);
        std::shared_ptr<SceneNode> getRoot() const;
        std::shared_ptr<SceneNode> addNode(std::shared_ptr<SceneNode> parent);
        std::shared_ptr<SceneNode> addNode();
        bool removeNode(std::shared_ptr<SceneNode> node);
        bool setParent(std::shared_ptr<SceneNode> parent, std::shared_ptr<SceneNode> child) const;
        void render();
        void render(std::shared_ptr<GLShader> overrideShader);
        std::shared_ptr<Camera> getCamera() const;
    private:
        friend class SceneNode;
        static bool isInTree(std::shared_ptr<SceneNode> tree, std::shared_ptr<SceneNode> node);
        void renderWithShader(std::shared_ptr<SceneNode> node, std::shared_ptr<GLShader> shader) const;
        void updateNodeShaderType(ShaderType previous, std::shared_ptr<SceneNode> node);
        std::shared_ptr<SceneNode> root{};
        std::map<ShaderType, std::vector<std::shared_ptr<SceneNode>>> shaderMap{};
        std::shared_ptr<Camera> camera{};
        std::shared_ptr<UboMvp> uboMvp{};
        std::shared_ptr<UboMaterial> uboMaterial{};
    };
}
