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

glm::vec3 shadow::Camera::getPosition() const
{
    return position;
}

bool shadow::Camera::isViewDirty() const
{
    return viewDirty;
}

bool shadow::Camera::isProjectionDirty() const
{
    return projectionDirty;
}

void shadow::Camera::setAspectRatio(float aspectRatio)
{
    this->aspectRatio = aspectRatio;
    projectionDirty = true;
}

void shadow::Camera::setFov(float fov)
{
    this->fov = fov;
    projectionDirty = true;
}

void shadow::Camera::setClip(float nearZ, float farZ)
{
    this->nearZ = nearZ;
    this->farZ = farZ;
    projectionDirty = true;
}

void shadow::Camera::setPosition(glm::vec3 position)
{
    this->position = position;
    viewDirty = true;
}

void shadow::Camera::setDirection(glm::vec3 direction)
{
    this->direction = direction;
    viewDirty = true;
}

void shadow::Camera::setUp(glm::vec3 up)
{
    this->up = up;
    viewDirty = true;
}
