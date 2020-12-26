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
        void resizeLights(GLsizei textureSize);
        void setBlurPasses(unsigned int blurPasses);
        double getTime() const;
        unsigned int getFps() const;
        unsigned int getBlurPasses() const;
        std::shared_ptr<Scene> getScene() const;
        std::shared_ptr<Camera> getCamera() const;
    private:
        AppWindow();
        void updateLightShadowSamplers();
        const char* GLSL_VERSION{ "#version 430" };
        GLsizei width{}, height{};
        glm::vec4 clearColor{ 0.0f, 0.0f, 0.0f, 1.0f };
        double currentTime{ 0.0 }, lastTime{ 0.0 };
        unsigned int fpsCounter{ 0U }, fpsSecond{ 1U }, measuredFps{ 0U }, blurPasses{ 1 };
        GLFWwindow* glfwWindow{ nullptr };
        std::shared_ptr<Camera> camera{};
        std::shared_ptr<Scene> scene{};
        std::shared_ptr<GLShader> ppShader{}, blurShader{}, depthDirShader{}, depthSpotShader{};
        std::shared_ptr<UboMvp> uboMvp{};
        std::shared_ptr<UboLights> uboLights{};
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
        } else
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
        scene->render(depthSpotShader);
        GL_POP_DEBUG_GROUP();

        GL_PUSH_DEBUG_GROUP("SpotLight");
        glBindFramebuffer(GL_FRAMEBUFFER, lightManager.getSpotFbo());
        glClear(GL_DEPTH_BUFFER_BIT);
        depthSpotShader->use();
        scene->render(depthSpotShader);
        GL_POP_DEBUG_GROUP();

        glCullFace(GL_BACK);

        GL_PUSH_DEBUG_GROUP("Gaussian blur (DirLight)");
        glDisable(GL_DEPTH_TEST);
        blurShader->use();
        glActiveTexture(GL_TEXTURE12);
        for (unsigned int i = 0; i < blurPasses; ++i)
        {
            glBindFramebuffer(GL_FRAMEBUFFER, lightManager.getTempFbo());
            blurShader->setVec2("direction", glm::vec2(1.0f, 0.0f));
            glBindTexture(GL_TEXTURE_2D, lightManager.getDirTexture());
            resourceManager.renderQuad();
            glBindFramebuffer(GL_FRAMEBUFFER, lightManager.getDirFbo());
            blurShader->setVec2("direction", glm::vec2(0.0f, 1.0f));
            glBindTexture(GL_TEXTURE_2D, lightManager.getTempTexture());
            resourceManager.renderQuad();
        }
        GL_POP_DEBUG_GROUP();

        GL_PUSH_DEBUG_GROUP("Gaussian blur (SpotLight)");
        for (unsigned int i = 0; i < blurPasses; ++i)
        {
            glBindFramebuffer(GL_FRAMEBUFFER, lightManager.getTempFbo());
            blurShader->setVec2("direction", glm::vec2(1.0f, 0.0f));
            glBindTexture(GL_TEXTURE_2D, lightManager.getSpotTexture());
            resourceManager.renderQuad();
            glBindFramebuffer(GL_FRAMEBUFFER, lightManager.getSpotFbo());
            blurShader->setVec2("direction", glm::vec2(0.0f, 1.0f));
            glBindTexture(GL_TEXTURE_2D, lightManager.getTempTexture());
            resourceManager.renderQuad();
        }
        glActiveTexture(GL_TEXTURE0);
        GL_POP_DEBUG_GROUP();

        GL_PUSH_DEBUG_GROUP("Main render");
        glEnable(GL_DEPTH_TEST);
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
