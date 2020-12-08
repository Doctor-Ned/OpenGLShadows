#include "AppWindow.h"
#include "ShadowLog.h"

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

shadow::AppWindow::~AppWindow()
{
    deinitialise();
}

bool shadow::AppWindow::initialise(GLsizei width, GLsizei height)
{
    if (width <= 0 || height <= 0)
    {
        SHADOW_CRITICAL("Window dimensions must be greater than zero!");
        return false;
    }
    glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);

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
        deinitialise();
        return false;
    }

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    (void)io;
    ImGui_ImplGlfw_InitForOpenGL(glfwWindow, true);
    ImGui_ImplOpenGL3_Init(GLSL_VERSION);
    ImGui::StyleColorsDark();

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

bool shadow::AppWindow::isInitialised() const
{
    return glfwWindow;
}

void shadow::AppWindow::deinitialise()
{
    if (glfwWindow)
    {
        glfwDestroyWindow(glfwWindow);
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
