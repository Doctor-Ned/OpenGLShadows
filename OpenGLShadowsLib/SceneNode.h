#pragma once

#include "Mesh.h"

#include <vector>
#include <memory>

namespace shadow
{
    class Scene;

    class SceneNode final
    {
    public:
        ~SceneNode() = default;
        SceneNode(SceneNode&) = delete;
        SceneNode(SceneNode&&) = delete;
        SceneNode& operator=(SceneNode&) = delete;
        SceneNode& operator=(SceneNode&&) = delete;
        bool isActive();
        bool isActiveSelf() const;
        glm::mat4 getModel() const;
        glm::mat4 getWorld();
        std::shared_ptr<Mesh> getMesh() const;
        SceneNode& setActiveSelf(bool activeSelf);
        SceneNode& setModel(glm::mat4 model);
        SceneNode& setMesh(std::shared_ptr<Mesh> mesh);
        SceneNode& translate(glm::vec3 vec);
        SceneNode& scale(glm::vec3 vec);
        SceneNode& rotate(float angle, glm::vec3 axis);
        std::shared_ptr<SceneNode> getParent() const;
        std::vector<std::shared_ptr<SceneNode>> getChildren() const;
    private:
        friend class Scene;
        SceneNode() = default;
        void setDirty();
        void setActiveDirty();
        std::weak_ptr<Scene> scene{};
        std::weak_ptr<SceneNode> parent{};
        std::shared_ptr<Mesh> mesh{};
        std::vector<std::shared_ptr<SceneNode>> children{};
        bool activeSelf{ true }, active{};
        bool dirty{ true }, activeDirty{ true };
        glm::mat4 model{ glm::mat4(1.0f) }, world{};
    };
}
