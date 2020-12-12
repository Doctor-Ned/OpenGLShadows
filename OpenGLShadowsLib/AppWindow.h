#pragma once

#include "ShadowLog.h"
#include "Camera.h"
#include "Scene.h"

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
        static AppWindow& getInstance();
        bool initialize(GLsizei width, GLsizei height, std::filesystem::path resourceDirectory);
        bool isInitialized() const;
        void deinitialize();
        inline bool shouldClose() const;
        void loop(double& timeDelta);
        void setClearColor(const glm::vec4& clearColor);
        void resize(GLsizei width, GLsizei height);
        std::shared_ptr<Scene> getScene() const;
        std::shared_ptr<Camera> getCamera() const;
    private:
        AppWindow();
        const char* GLSL_VERSION{ "#version 430" };
        GLsizei width{}, height{};
        glm::vec4 clearColor{ 0.0f, 0.0f, 0.0f, 1.0f };
        double currentTime{}, lastTime{};
        GLFWwindow* glfwWindow{ nullptr };
        std::shared_ptr<Camera> camera{};
        std::shared_ptr<Scene> scene{};
    };

    inline bool AppWindow::shouldClose() const
    {
        return glfwWindowShouldClose(glfwWindow);
    }

}

