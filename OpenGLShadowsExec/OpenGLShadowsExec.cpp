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
    std::shared_ptr<Camera> camera = appWindow.getCamera();
    //std::shared_ptr<ModelMesh> modelFlareGun = resourceManager.getModel("FlareGun/FlareGun.obj");
    std::shared_ptr<ModelMesh> modelBackpack = resourceManager.getModel("Backpack/backpack.obj");
    //std::shared_ptr<MaterialModelMesh> modelCat = resourceManager.getMaterialModel("OrigamiCat/OrigamiCat.obj", std::make_shared<Material>(glm::vec3(1.0f, 1.0f, 1.0f), 1.0f, 0.0f));
    std::shared_ptr<Scene> scene = appWindow.getScene();
    std::shared_ptr<SceneNode> node = scene->addNode();
    camera->setPosition(glm::vec3(0.0f, 0.0f, 1.0f));
    node->setMesh(modelBackpack);
    node->scale(glm::vec3(0.1f));
    double timeDelta = 0.0;
    while (!appWindow.shouldClose())
    {
        appWindow.loop(timeDelta);
        node->rotate(static_cast<float>(timeDelta) * 0.25f, glm::vec3(0.0f, 1.0f, 0.0f));
        resourceManager.updateShaders();
    }
    return 0;
}
