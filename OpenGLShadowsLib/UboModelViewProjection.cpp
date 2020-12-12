#include "UboModelViewProjection.h"

#include <glm/gtc/type_ptr.hpp>

shadow::UboModelViewProjection::UboModelViewProjection() : UniformBufferObject("ModelViewProjection", 0) {}

void shadow::UboModelViewProjection::setModel(glm::mat4& model)
{
    bufferSubData(value_ptr(model), sizeof(glm::mat4), offsetof(ModelViewProjection, model));
}

void shadow::UboModelViewProjection::setView(glm::mat4& view)
{
    bufferSubData(value_ptr(view), sizeof(glm::mat4), offsetof(ModelViewProjection, view));
}

void shadow::UboModelViewProjection::setProjection(glm::mat4& projection)
{
    bufferSubData(value_ptr(projection), sizeof(glm::mat4), offsetof(ModelViewProjection, projection));
}

void shadow::UboModelViewProjection::setViewPosition(glm::vec3& viewPosition)
{
    bufferSubData(value_ptr(viewPosition), sizeof(glm::vec3), offsetof(ModelViewProjection, viewPosition));
}
