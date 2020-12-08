#pragma once

#include "glad/glad.h"
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

namespace shadow
{
    class AppWindow final
    {
    public:
        ~AppWindow();
        AppWindow(AppWindow&) = delete;
        AppWindow(AppWindow&&) = delete;
        AppWindow& operator=(AppWindow&) = delete;
        AppWindow& operator=(AppWindow&&) = delete;
        static AppWindow* getInstance();
        bool initialize(GLsizei width, GLsizei height);
        bool isInitialized() const;
        void deinitialize();
        inline bool shouldClose() const;
        void loop();
        void setClearColor(const glm::vec4& clearColor);
        void resize(GLsizei width, GLsizei height);
    private:
        AppWindow();
        const char* GLSL_VERSION{ "#version 430" };
        GLsizei width{}, height{};
        glm::vec4 clearColor{ 0.0f, 0.0f, 0.0f, 1.0f };
        double currentTime{}, lastTime{}, timeDelta{};
        GLFWwindow* glfwWindow{ nullptr };
    };

    inline bool AppWindow::shouldClose() const
    {
        return glfwWindowShouldClose(glfwWindow);
    }

}

