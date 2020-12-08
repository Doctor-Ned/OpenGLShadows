#include "AppWindow.h"
#include "ShadowLog.h"

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

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

shadow::AppWindow* shadow::AppWindow::getInstance()
{
    static AppWindow appWindow{};
    return &appWindow;
}

bool shadow::AppWindow::initialize(GLsizei width, GLsizei height)
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

    this->width = width;
    this->height = height;
    currentTime = 0.0;
    lastTime = 0.0;
    timeDelta = 0.0;

    return true;
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

void shadow::AppWindow::loop()
{
    glfwPollEvents();
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
    currentTime = glfwGetTime();
    timeDelta = currentTime - lastTime;
    lastTime = currentTime;
    glViewport(0, 0, width, height);
    glClearColor(clearColor.x, clearColor.y, clearColor.z, clearColor.w);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    ImGui::ShowDemoWindow();
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    glfwSwapBuffers(glfwWindow);
}

void shadow::AppWindow::setClearColor(const glm::vec4& clearColor)
{
    this->clearColor = clearColor;
}

void shadow::AppWindow::resize(GLsizei width, GLsizei height)
{
    if (isInitialized())
    {
        SHADOW_DEBUG("Changing window size to {}x{}...", width, height);
        glfwSetWindowSize(glfwWindow, width, height);
        this->width = width;
        this->height = height;
    }
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
