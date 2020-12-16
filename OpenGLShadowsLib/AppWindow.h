#pragma once

#include "ShadowLog.h"
#include "Camera.h"
#include "Scene.h"

#include "glad/glad.h"
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

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
        template<typename F>
        void loop(double& timeDelta, F& guiProc);
        void setClearColor(const glm::vec4& clearColor);
        void resize(GLsizei width, GLsizei height);
        double getTime() const;
        unsigned int getFps() const;
        std::shared_ptr<Scene> getScene() const;
        std::shared_ptr<Camera> getCamera() const;
    private:
        AppWindow();
        const char* GLSL_VERSION{ "#version 430" };
        GLsizei width{}, height{};
        glm::vec4 clearColor{ 0.0f, 0.0f, 0.0f, 1.0f };
        double currentTime{ 0.0 }, lastTime{ 0.0 };
        unsigned int fpsCounter{ 0U }, fpsSecond{ 1U }, measuredFps{ 0U };
        GLFWwindow* glfwWindow{ nullptr };
        std::shared_ptr<Camera> camera{};
        std::shared_ptr<Scene> scene{};
    };

    inline bool AppWindow::shouldClose() const
    {
        return glfwWindowShouldClose(glfwWindow);
    }

    template<typename F>
    void shadow::AppWindow::loop(double& timeDelta, F& guiProc)
    {
        assert(glfwWindow);
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
        currentTime = glfwGetTime();
        timeDelta = currentTime - lastTime;
        if (static_cast<unsigned int>(currentTime) >= fpsSecond)
        {
            measuredFps = fpsCounter;
            fpsCounter = 0U;
            ++fpsSecond;
        } else
        {
            ++fpsCounter;
        }
        lastTime = currentTime;
        glViewport(0, 0, width, height);
        glClearColor(clearColor.x, clearColor.y, clearColor.z, clearColor.w);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        scene->render();
        //ImGui::ShowDemoWindow();
        guiProc();
        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        glfwSwapBuffers(glfwWindow);
        glfwPollEvents();
    }
}

