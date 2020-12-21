#include "AppWindow.h"

#include "ShadowUtils.h"

static void glfw_error_callback(int error, const char* description)
{
    SHADOW_ERROR("GLFW error #{}: {}", error, description);
}

shadow::AppWindow::~AppWindow()
{
    try
    {
        SHADOW_DEBUG("Destroying window...");
        deinitialize();
        SHADOW_DEBUG("Terminating GLFW...");
        glfwTerminate();
    } catch (std::exception& e)
    {
        SHADOW_ERROR("Window destruction failed! {}", e.what());
    }
}

shadow::AppWindow& shadow::AppWindow::getInstance()
{
    static AppWindow appWindow{};
    return appWindow;
}

bool shadow::AppWindow::initialize(GLsizei width, GLsizei height, GLsizei lightTextureSize, std::filesystem::path resourceDirectory)
{
    if (width <= 0 || height <= 0)
    {
        SHADOW_CRITICAL("Window dimensions must be greater than zero!");
        return false;
    }
    SHADOW_DEBUG("Initializing a {}x{} window...", width, height);
    glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);

    SHADOW_DEBUG("Creating GLFW context...");
    glfwWindow = glfwCreateWindow(width, height, "Shadows", nullptr, nullptr);
    if (glfwWindow == nullptr)
    {
        SHADOW_CRITICAL("Failed to create window!");
        return false;
    }
    glfwMakeContextCurrent(glfwWindow);
    glfwSwapInterval(0); // disable v-sync

    if (!gladLoadGLLoader(reinterpret_cast<GLADloadproc>(glfwGetProcAddress)))
    {
        SHADOW_CRITICAL("Failed to initialize OpenGL loader!");
        deinitialize();
        return false;
    }

    SHADOW_DEBUG("Initializing ImGui...");
    try
    {
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGuiIO& io = ImGui::GetIO();
        (void)io;
        ImGui_ImplGlfw_InitForOpenGL(glfwWindow, true);
        ImGui_ImplOpenGL3_Init(GLSL_VERSION);
        ImGui::StyleColorsDark();
    } catch (std::exception& e)
    {
        SHADOW_CRITICAL("ImGui initialization failed! {}", e.what());
        deinitialize();
        return false;
    }

    glEnable(GL_DEPTH_TEST);
    glFrontFace(GL_CCW);
    glCullFace(GL_BACK);
    glEnable(GL_BLEND);
    glClearColor(clearColor.x, clearColor.y, clearColor.z, clearColor.w);

    if (!mainFramebuffer.initialize(true, GL_COLOR_ATTACHMENT0, GL_RGBA16F, width, height, GL_RGBA, GL_FLOAT))
    {
        return false;
    }

    if (!LightManager::getInstance().initialize(lightTextureSize))
    {
        return false;
    }

    ResourceManager& resourceManager = ResourceManager::getInstance();
    if (!resourceManager.initialize(resourceDirectory))
    {
        return false;
    }

    this->ppShader = resourceManager.getShader(ShaderType::PostProcess);
    this->depthShader = resourceManager.getShader(ShaderType::Depth);
    this->uboMvp = resourceManager.getUboMvp();
    this->uboLights = resourceManager.getUboLights();
    this->dirLight = uboLights->getDirectionalLight();
    this->spotLight = uboLights->getSpotLight();
    this->width = width;
    this->height = height;

    updateLightShadowSamplers();

    camera = std::make_shared<Camera>(
        static_cast<float>(width) / static_cast<float>(height),
        FPI * 0.25f,
        0.01f,
        100.0f,
        glm::vec3(0.0f, 0.0f, 0.0f),
        glm::vec3(0.0f, 0.0f, -1.0f),
        glm::vec3(0.0f, 1.0f, 0.0f)
        );

    scene = std::make_shared<Scene>();
    return scene->initialize(camera);
}

bool shadow::AppWindow::isInitialized() const
{
    return glfwWindow;
}

void shadow::AppWindow::deinitialize()
{
    SHADOW_DEBUG("Deinitializing window...");
    if (glfwWindow)
    {
        SHADOW_DEBUG("Destroying GLFW context...");
        try
        {
            glfwDestroyWindow(glfwWindow);
        } catch (std::exception& e)
        {
            SHADOW_ERROR("Failed to destroy GLFW context! {}", e.what());
        }
        glfwWindow = nullptr;
    }
    width = height = 0;
}

void shadow::AppWindow::setClearColor(const glm::vec4& clearColor)
{
    this->clearColor = clearColor;
}

void shadow::AppWindow::resize(GLsizei width, GLsizei height)
{
    assert(glfwWindow);
    SHADOW_DEBUG("Changing window size to {}x{}...", width, height);
    glfwSetWindowSize(glfwWindow, width, height);
    this->width = width;
    this->height = height;
    mainFramebuffer.resize(width, height);
    camera->setAspectRatio(static_cast<float>(width) / static_cast<float>(height));
}

void shadow::AppWindow::resizeLights(GLsizei textureSize)
{
    LightManager::getInstance().resize(textureSize);
    updateLightShadowSamplers();
}

double shadow::AppWindow::getTime() const
{
    return currentTime;
}

unsigned int shadow::AppWindow::getFps() const
{
    return measuredFps;
}

std::shared_ptr<shadow::Scene> shadow::AppWindow::getScene() const
{
    return scene;
}

std::shared_ptr<shadow::Camera> shadow::AppWindow::getCamera() const
{
    return camera;
}

shadow::AppWindow::AppWindow()
{
    SHADOW_DEBUG("Initializing GLFW...");
    glfwSetErrorCallback(glfw_error_callback);
    if (!glfwInit())
    {
        SHADOW_CRITICAL("GLFW initialisation failed!");
    }
}

void shadow::AppWindow::updateLightShadowSamplers()
{
    ResourceManager& resourceManager = ResourceManager::getInstance();
    std::vector<std::shared_ptr<GLShader>> shaders{
        resourceManager.getShader(ShaderType::Material),
        resourceManager.getShader(ShaderType::Texture)
    };
    LightManager& lightManager = LightManager::getInstance();
    for (const std::shared_ptr<GLShader>& shader : shaders)
    {
        shader->use();
        glActiveTexture(GL_TEXTURE10);
        glBindTexture(GL_TEXTURE_2D, lightManager.getDirTexture());
        glActiveTexture(GL_TEXTURE11);
        glBindTexture(GL_TEXTURE_2D, lightManager.getSpotTexture());
    }
    glActiveTexture(GL_TEXTURE0);
}
