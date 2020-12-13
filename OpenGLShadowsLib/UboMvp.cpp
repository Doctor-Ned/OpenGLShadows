#include "UboMvp.h"

#include <glm/gtc/type_ptr.hpp>

shadow::UboMvp::UboMvp() : UniformBufferObject("ModelViewProjection", 0) {}

void shadow::UboMvp::setModel(glm::mat4& model)
{
    bufferSubData(value_ptr(model), sizeof(glm::mat4), offsetof(UboMvpStruct, model));
}

void shadow::UboMvp::setView(glm::mat4& view)
{
    bufferSubData(value_ptr(view), sizeof(glm::mat4), offsetof(UboMvpStruct, view));
}

void shadow::UboMvp::setProjection(glm::mat4& projection)
{
    bufferSubData(value_ptr(projection), sizeof(glm::mat4), offsetof(UboMvpStruct, projection));
}

void shadow::UboMvp::setViewPosition(glm::vec3& viewPosition)
{
    bufferSubData(value_ptr(viewPosition), sizeof(glm::vec3), offsetof(UboMvpStruct, viewPosition));
}
