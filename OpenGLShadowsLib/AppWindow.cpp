#include "AppWindow.h"

#include "ShadowUtils.h"

#include <stb_image_write.h>

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
    }
    catch (std::exception& e)
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
    }
    catch (std::exception& e)
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

    if (!mainFramebuffer.initialize(true, GL_COLOR_ATTACHMENT0, GL_RGBA16F, width, height, GL_RGBA, GL_FLOAT, GL_NEAREST, GL_REPEAT))
    {
        return false;
    }

    ResourceManager& resourceManager = ResourceManager::getInstance();
    if (!resourceManager.initialize(resourceDirectory, width, height))
    {
        return false;
    }

#if SHADOW_MASTER || SHADOW_CHSS
    if (!LightManager::getInstance().initialize(lightTextureSize, width, height))
    {
        return false;
    }
#else
    if (!LightManager::getInstance().initialize(lightTextureSize))
    {
        return false;
    }
#endif

    this->ppShader = resourceManager.getShader(ShaderType::PostProcess);
#if SHADOW_VSM
    this->depthDirShader = resourceManager.getShader(ShaderType::DepthDirVSM);
    this->depthSpotShader = resourceManager.getShader(ShaderType::DepthSpotVSM);
    this->blurShader = resourceManager.getShader(ShaderType::GaussianBlur);
#else
    this->depthDirShader = resourceManager.getShader(ShaderType::DepthDir);
    this->depthSpotShader = resourceManager.getShader(ShaderType::DepthSpot);
#endif
#if SHADOW_MASTER || SHADOW_CHSS
    this->dirPenumbraShader = resourceManager.getShader(ShaderType::DirPenumbra);
    this->spotPenumbraShader = resourceManager.getShader(ShaderType::SpotPenumbra);
#endif
    this->uboMvp = resourceManager.getUboMvp();
    this->uboLights = resourceManager.getUboLights();
    this->uboWindow = resourceManager.getUboWindow();
    this->dirLight = uboLights->getDirectionalLight();
    this->spotLight = uboLights->getSpotLight();
    this->width = width;
    this->height = height;

    glm::vec2 windowSize{ width, height };
    uboWindow->setWindowSize(windowSize);

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
        }
        catch (std::exception& e)
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
    glm::vec2 windowSize{ width, height };
    uboWindow->setWindowSize(windowSize);
}

#if SHADOW_MASTER || SHADOW_CHSS
void shadow::AppWindow::resizeLights(GLsizei textureSize, unsigned int penumbraTextureSizeDivisor)
{
    assert(penumbraTextureSizeDivisor);
    LightManager::getInstance().resize(textureSize, width / penumbraTextureSizeDivisor, height / penumbraTextureSizeDivisor);
    updateLightShadowSamplers();
}
#else
void shadow::AppWindow::resizeLights(GLsizei textureSize)
{
    LightManager::getInstance().resize(textureSize);
    updateLightShadowSamplers();
}
#endif

#if SHADOW_VSM
void shadow::AppWindow::setBlurPasses(unsigned int blurPasses)
{
    this->blurPasses = blurPasses;
}

unsigned int shadow::AppWindow::getBlurPasses() const
{
    return blurPasses;
}
#endif

void shadow::AppWindow::takeScreenshot(const std::filesystem::path& filePath) const
{
    if (filePath.has_parent_path() && !std::filesystem::exists(filePath.parent_path())) {
        std::filesystem::create_directories(filePath.parent_path());
    }
    const std::string FORMAT = ".tga";
    const int CHANNELS = 3;
    const int cwidth = width * CHANNELS;
    const size_t pixelCount = cwidth * height;
    unsigned char* pixels = new unsigned char[pixelCount];

    glPixelStorei(GL_PACK_ALIGNMENT, 1);
    glReadBuffer(GL_FRONT);
    glReadPixels(0, 0, width, height, GL_RGB, GL_UNSIGNED_BYTE, pixels);

    for (int i = 0; i < width; ++i)
    {
        for (int j = 0; j < height / 2; ++j)
        {
            unsigned char pix[CHANNELS];
            int mirror = height - 1 - j;
            memcpy(pix, pixels + j * cwidth + i * CHANNELS, CHANNELS);
            memcpy(pixels + j * cwidth + i * CHANNELS, pixels + mirror * cwidth + i * CHANNELS, CHANNELS);
            memcpy(pixels + mirror * cwidth + i * CHANNELS, pix, CHANNELS);
        }
    }

    stbi_write_tga((filePath.generic_string() + FORMAT).c_str(), width, height, CHANNELS, pixels);
    delete[] pixels;
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
#if SHADOW_MASTER || SHADOW_CHSS
        glActiveTexture(GL_TEXTURE10);
        glBindTexture(GL_TEXTURE_2D, lightManager.getDirTexture());
        glActiveTexture(GL_TEXTURE11);
        glBindTexture(GL_TEXTURE_2D, lightManager.getDirPenumbraTexture());
        glActiveTexture(GL_TEXTURE12);
        glBindTexture(GL_TEXTURE_2D, lightManager.getSpotTexture());
        glActiveTexture(GL_TEXTURE13);
        glBindTexture(GL_TEXTURE_2D, lightManager.getSpotPenumbraTexture());
#else
        glActiveTexture(GL_TEXTURE10);
        glBindTexture(GL_TEXTURE_2D, lightManager.getDirTexture());
        glActiveTexture(GL_TEXTURE11);
        glBindTexture(GL_TEXTURE_2D, lightManager.getSpotTexture());
#endif
    }
    glActiveTexture(GL_TEXTURE0);
#if SHADOW_VSM
    this->blurShader->use();
    this->blurShader->setVec2("resolution", glm::vec2(lightManager.getTextureSize(), lightManager.getTextureSize()));
#endif
}
