#include "UboWindow.h"

#include <glm/gtc/type_ptr.hpp>

shadow::UboWindow::UboWindow() : UniformBufferObject("Window", 3) {}

void shadow::UboWindow::setWindowSize(glm::vec2& windowSize)
{
    bufferSubData(value_ptr(windowSize), sizeof(glm::vec2), offsetof(UboWindowStruct, windowSize));
}
