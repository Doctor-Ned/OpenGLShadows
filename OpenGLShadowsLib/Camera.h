#pragma once

#include <glm/glm.hpp>

namespace shadow
{
    class Camera final
    {
    public:
        Camera(float aspectRatio, float fov, float nearZ, float farZ, glm::vec3 position, glm::vec3 direction, glm::vec3 up);
        ~Camera() = default;
        Camera(Camera&) = delete;
        Camera(Camera&&) = delete;
        Camera& operator=(Camera&) = delete;
        Camera& operator=(Camera&&) = delete;
        glm::mat4 getView();
        glm::mat4 getProjection();
        glm::vec3 getPosition() const;
        bool isViewDirty() const;
        bool isProjectionDirty() const;
        void setAspectRatio(float aspectRatio);
        void setFov(float fov);
        void setClip(float nearZ, float farZ);
        void setPosition(glm::vec3 position);
        void setDirection(glm::vec3 direction);
        void setUp(glm::vec3 up);
        //todo: add methods to modify the view/projection
    private:
        bool viewDirty{ true }, projectionDirty{ true };
        glm::mat4 view{}, projection{};
        float aspectRatio{}, fov{}, nearZ{}, farZ{};
        glm::vec3 position{}, direction{}, up{};
    };
}
