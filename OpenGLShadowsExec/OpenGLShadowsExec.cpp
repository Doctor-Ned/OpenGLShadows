#include "AppWindow.h"
#include "ResourceManager.h"

int main()
{
    if (!shadow::ResourceManager::getInstance().initialize("../../Resources"))
    {
        return 1;
    }
    shadow::AppWindow& appWindow = shadow::AppWindow::getInstance();
    if (!appWindow.initialize(1280, 720))
    {
        return 2;
    }
    while (!appWindow.shouldClose())
    {
        appWindow.loop();
    }
    return 0;
}
