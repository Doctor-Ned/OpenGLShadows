#include "AppWindow.h"
#include "ResourceManager.h"
#include "MaterialModelMesh.h"
#include "SceneNode.h"

int main()
{
    using namespace shadow;
    AppWindow& appWindow = AppWindow::getInstance();
    ResourceManager& resourceManager = ResourceManager::getInstance();
    if (!appWindow.initialize(1280, 720, "../../Resources"))
    {
        return 1;
    }
    std::shared_ptr<ModelMesh> modelFlareGun = resourceManager.getModel("FlareGun/Flare_Long.obj");
    //todo: add MaterialModelMesh support in ResourceManager! create a transition structure of all model data than gets loaded once and used appropriately
    std::shared_ptr<Scene> scene = appWindow.getScene();
    std::shared_ptr<SceneNode> node = scene->addNode();
    node->setMesh(modelFlareGun);
    node->scale(glm::vec3(0.01f));
    while (!appWindow.shouldClose())
    {
        appWindow.loop();
        resourceManager.updateShaders();
    }
    return 0;
}
