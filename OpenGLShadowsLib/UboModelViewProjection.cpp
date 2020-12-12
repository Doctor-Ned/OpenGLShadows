#include "UboModelViewProjection.h"

shadow::UboModelViewProjection::UboModelViewProjection() : UniformBufferObject("ModelViewProjection", 0) {}

void shadow::UboModelViewProjection::setModel(glm::mat4& model)
{
    bufferSubData(&model, sizeof(glm::mat4), offsetof(ModelViewProjection, model));
}

void shadow::UboModelViewProjection::setView(glm::mat4& view)
{
    bufferSubData(&view, sizeof(glm::mat4), offsetof(ModelViewProjection, view));
}

void shadow::UboModelViewProjection::setProjection(glm::mat4& projection)
{
    bufferSubData(&projection, sizeof(glm::mat4), offsetof(ModelViewProjection, projection));
}
