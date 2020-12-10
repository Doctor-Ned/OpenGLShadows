#include "AppWindow.h"
#include "ResourceManager.h"

int main()
{
    shadow::AppWindow& appWindow = shadow::AppWindow::getInstance();
    if (appWindow.initialize(1280, 720))
    {
        auto model = shadow::ResourceManager::getInstance().getModel("../Resources/Models/FlareGun/Flare_Short.obj");
        while (!appWindow.shouldClose())
        {
            appWindow.loop();
        }
    }
    return 0;
}
