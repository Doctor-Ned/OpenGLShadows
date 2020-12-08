#include "ShadowLog.h"
#include "AppWindow.h"

#include "glad/glad.h"
#include <GLFW/glfw3.h>

static void glfw_error_callback(int error, const char* description)
{
    SHADOW_ERROR("GLFW error #{}: {}", error, description);
}

int main()
{
    glfwSetErrorCallback(glfw_error_callback);
    if (!glfwInit())
    {
        SHADOW_CRITICAL("GLFW initialisation failed!");
        return 1;
    }
    shadow::AppWindow appWindow{};
    if (appWindow.initialise(1280, 720))
    {
        while (!appWindow.shouldClose())
        {
            appWindow.loop();
        }
    }
    glfwTerminate();
    return 0;
}
