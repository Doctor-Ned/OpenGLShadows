#include "AppWindow.h"

int main()
{
    shadow::AppWindow* appWindow = shadow::AppWindow::getInstance();
    if (appWindow->initialize(1280, 720))
    {
        while (!appWindow->shouldClose())
        {
            appWindow->loop();
        }
    }
    return 0;
}
