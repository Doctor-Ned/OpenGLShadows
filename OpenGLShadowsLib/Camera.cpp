#include "Camera.h"
#include <glm/ext/matrix_transform.hpp>
#include <glm/ext/matrix_clip_space.hpp>

shadow::Camera::Camera(float aspectRatio, float fov, float nearZ, float farZ,
                       glm::vec3 position, glm::vec3 direction, glm::vec3 up)
    :aspectRatio(aspectRatio), fov(fov), nearZ(nearZ), farZ(farZ),
    position(position), direction(normalize(direction)), up(normalize(up))
{}

glm::mat4 shadow::Camera::getView()
{
    if (viewDirty)
    {
        view = lookAt(position, position + direction, up);
        viewDirty = false;
    }
    return view;
}

glm::mat4 shadow::Camera::getProjection()
{
    if (projectionDirty)
    {
        projection = glm::perspective(fov, aspectRatio, nearZ, farZ);
        projectionDirty = false;
    }
    return projection;
}

bool shadow::Camera::isViewDirty() const
{
    return viewDirty;
}

bool shadow::Camera::isProjectionDirty() const
{
    return projectionDirty;
}
