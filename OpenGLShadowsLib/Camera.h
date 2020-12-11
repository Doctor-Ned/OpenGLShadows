#pragma once

#include <glm/glm.hpp>

namespace shadow
{
    class Camera final
    {
    public:
        Camera(float aspectRatio, float fov, float near, float far, glm::vec3 position, glm::vec3 direction, glm::vec3 up);
        ~Camera() = default;
        Camera(Camera&) = delete;
        Camera(Camera&&) = delete;
        Camera& operator=(Camera&) = delete;
        Camera& operator=(Camera&&) = delete;
        glm::mat4 getView();
        glm::mat4 getProjection();
        //todo: add methods to modify the view/projection
    private:
        bool viewDirty{ true }, projectionDirty{ true };
        glm::mat4 view{}, projection{};
        float aspectRatio{}, fov{}, near{}, far{};
        glm::vec3 position{}, direction{}, up{};
    };
}
