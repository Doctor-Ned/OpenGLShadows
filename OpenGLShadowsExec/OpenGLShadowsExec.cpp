#include "AppWindow.h"
#include "ResourceManager.h"

int main()
{
    shadow::ResourceManager::getInstance().initialize("../../../Resources");
    shadow::AppWindow& appWindow = shadow::AppWindow::getInstance();
    if (appWindow.initialize(1280, 720))
    {
        while (!appWindow.shouldClose())
        {
            appWindow.loop();
        }
    }
    return 0;
}
