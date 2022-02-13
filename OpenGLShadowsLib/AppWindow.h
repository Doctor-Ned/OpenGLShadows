#pragma once

#include "ShadowLog.h"
#include "GLDebug.h"
#include "Camera.h"
#include "Scene.h"
#include "Framebuffer.h"
#include "ResourceManager.h"
#include "LightManager.h"

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
        bool initialize(GLsizei width, GLsizei height, GLsizei lightTextureSize, std::filesystem::path resourceDirectory);
        bool isInitialized() const;
        void deinitialize();
        inline bool shouldClose() const;
        template<typename F>
        void loop(double& timeDelta, F& guiProc);
        void setClearColor(const glm::vec4& clearColor);
        void resize(GLsizei width, GLsizei height);
#if SHADOW_MASTER || SHADOW_CHSS
        void resizeLights(GLsizei textureSize, unsigned int penumbraTextureSizeDivisor);
#else
        void resizeLights(GLsizei textureSize);
#endif
        double getTime() const;
        unsigned int getFps() const;
        std::shared_ptr<Scene> getScene() const;
        std::shared_ptr<Camera> getCamera() const;
    private:
        AppWindow();
        void updateLightShadowSamplers();
        const char* GLSL_VERSION{ "#version 430" };
        GLsizei width{}, height{};
        glm::vec4 clearColor{ 0.0f, 0.0f, 0.0f, 1.0f };
        double currentTime{ 0.0 }, lastTime{ 0.0 };
        unsigned int fpsCounter{ 0U }, fpsSecond{ 1U }, measuredFps{ 0U };
        GLFWwindow* glfwWindow{ nullptr };
        std::shared_ptr<Camera> camera{};
        std::shared_ptr<Scene> scene{};
        std::shared_ptr<GLShader> ppShader{}, depthDirShader{}, depthSpotShader{};
#if SHADOW_MASTER || SHADOW_CHSS
        std::shared_ptr<GLShader> dirPenumbraShader{}, spotPenumbraShader{};
#endif
        std::shared_ptr<UboMvp> uboMvp{};
        std::shared_ptr<UboLights> uboLights{};
        std::shared_ptr<UboWindow> uboWindow{};
        std::shared_ptr<DirectionalLight> dirLight{};
        std::shared_ptr<SpotLight> spotLight{};
        Framebuffer mainFramebuffer{};
    };

    inline bool AppWindow::shouldClose() const
    {
        return glfwWindowShouldClose(glfwWindow);
    }

    template<typename F>
    void AppWindow::loop(double& timeDelta, F& guiProc)
    {
        assert(glfwWindow);
        ResourceManager& resourceManager = ResourceManager::getInstance();
        LightManager& lightManager = LightManager::getInstance();
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
        }
        else
        {
            ++fpsCounter;
        }
        lastTime = currentTime;
        uboLights->update();
        glEnable(GL_DEPTH_TEST);
        glCullFace(GL_FRONT);

        GL_PUSH_DEBUG_GROUP("DirLight");
        glViewport(0, 0, lightManager.getTextureSize(), lightManager.getTextureSize());
        glBindFramebuffer(GL_FRAMEBUFFER, lightManager.getDirFbo());
        glClear(GL_DEPTH_BUFFER_BIT);
        depthDirShader->use();
        scene->render(depthDirShader);
        GL_POP_DEBUG_GROUP();

        GL_PUSH_DEBUG_GROUP("SpotLight");
        glBindFramebuffer(GL_FRAMEBUFFER, lightManager.getSpotFbo());
        glClear(GL_DEPTH_BUFFER_BIT);
        depthSpotShader->use();
        scene->render(depthSpotShader);
        GL_POP_DEBUG_GROUP();

#if SHADOW_MASTER || SHADOW_CHSS
        GL_PUSH_DEBUG_GROUP("DirLightPenumbra");
        glViewport(0, 0, lightManager.getPenumbraTextureWidth(), lightManager.getPenumbraTextureHeight());
        glBindFramebuffer(GL_FRAMEBUFFER, lightManager.getDirPenumbraFbo());
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        dirPenumbraShader->use();
        scene->render(dirPenumbraShader);
        GL_POP_DEBUG_GROUP();

        GL_PUSH_DEBUG_GROUP("SpotLightPenumbra");
        glBindFramebuffer(GL_FRAMEBUFFER, lightManager.getSpotPenumbraFbo());
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        spotPenumbraShader->use();
        scene->render(spotPenumbraShader);
        GL_POP_DEBUG_GROUP();
#endif

        glCullFace(GL_BACK);

        GL_PUSH_DEBUG_GROUP("Main render");
        glViewport(0, 0, width, height);
        glBindFramebuffer(GL_FRAMEBUFFER, mainFramebuffer.getFbo());
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        if (camera->isViewDirty())
        {
            glm::mat4 view = camera->getView();
            glm::vec3 viewPosition = camera->getPosition();
            uboMvp->setView(view);
            uboMvp->setViewPosition(viewPosition);
        }
        if (camera->isProjectionDirty())
        {
            glm::mat4 projection = camera->getProjection();
            uboMvp->setProjection(projection);
        }
        scene->render();
        GL_POP_DEBUG_GROUP();

        GL_PUSH_DEBUG_GROUP("PostProcess");
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glDisable(GL_DEPTH_TEST);
        glClear(GL_COLOR_BUFFER_BIT);
        ppShader->use();
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, mainFramebuffer.getTexture());
        resourceManager.renderQuad();
        GL_POP_DEBUG_GROUP();

        GL_PUSH_DEBUG_GROUP("GUI");
        //ImGui::ShowDemoWindow();
        guiProc();
        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        GL_POP_DEBUG_GROUP();

        glfwSwapBuffers(glfwWindow);
        glfwPollEvents();
    }
}
