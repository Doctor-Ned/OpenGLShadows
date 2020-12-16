#include "AppWindow.h"
#include "ResourceManager.h"
#include "MaterialModelMesh.h"
#include "SceneNode.h"
#include "Primitives.h"
#include "ShadowUtils.h"

#include <glm/gtx/transform.hpp>
#include <glm/ext/quaternion_trigonometric.inl>

int main()
{
    using namespace shadow;
    AppWindow& appWindow = AppWindow::getInstance();
    ResourceManager& resourceManager = ResourceManager::getInstance();
    if (!appWindow.initialize(1920, 1080, "../../Resources"))
    {
        return 1;
    }
    std::shared_ptr<UboLights> uboLights = ResourceManager::getInstance().getUboLights();
    std::shared_ptr<DirectionalLight> dirLight = uboLights->getDirectionalLight();
    std::shared_ptr<SpotLight> spotLight = uboLights->getSpotLight();
    uboLights->setAmbient(0.1f);
    dirLight->setColor(glm::vec3(1.0f, 1.0f, 1.0f));
    //dirLight->setStrength(5.0f);
    spotLight->setColor(glm::vec3(1.0f, 1.0f, 1.0f));
    //spotLight->setStrength(1.0f);
    spotLight->setInnerCutOff(cosf(FPI * 0.5f));
    spotLight->setOuterCutOff(cosf(FPI));
    spotLight->setPosition(glm::vec3(0.0f, 5.0f, 0.0f));
    std::shared_ptr<Camera> camera = appWindow.getCamera();
    std::shared_ptr<PrimitiveData> planeData = Primitives::plane(5.0f, 5.0f, glm::vec2(5.0f));
    std::shared_ptr<TextureMesh> plane = std::make_shared<TextureMesh>(
        planeData->toTextureVertex(), planeData->getIndices(),
        std::map <TextureType, std::shared_ptr<Texture>> {
            { TextureType::Albedo, resourceManager.getTexture("Planks/planks_albedo.png") },
            { TextureType::Roughness, resourceManager.getTexture("Planks/planks_roughness.png") },
            { TextureType::Metalness, resourceManager.getTexture("Planks/planks_metallic.png") },
            { TextureType::Normal, resourceManager.getTexture("Planks/planks_normal.png") }
    });
    //std::shared_ptr<ModelMesh> modelFlareGun = resourceManager.getModel("FlareGun/FlareGun.obj");
    //std::shared_ptr<ModelMesh> modelBackpack = resourceManager.getModel("Backpack/backpack.obj");
    //std::shared_ptr<MaterialModelMesh> modelCat = resourceManager.getMaterialModel("OrigamiCat/OrigamiCat.obj", std::make_shared<Material>(glm::vec3(1.0f, 1.0f, 1.0f), 1.0f, 0.2f));
    std::shared_ptr<ModelMesh> modelGun = resourceManager.getModel("Gun/gun.obj");
    std::shared_ptr<ModelMesh> table = resourceManager.getModel("Table/Table.obj");
    std::shared_ptr<ModelMesh> suitcase = resourceManager.getModel("Suitcase/Vintage_Suitcase_LP.obj");
    std::shared_ptr<ModelMesh> chair = resourceManager.getModel("Chair/Chair.obj");
    std::shared_ptr<Scene> scene = appWindow.getScene();
    std::shared_ptr<SceneNode> node = scene->addNode(), suitcaseNode = scene->addNode(), chairNode = scene->addNode(), planeNode = scene->addNode(), tableNode = scene->addNode();
    camera->setPosition(glm::vec3(0.0f, 1.0f, 1.0f));
    camera->setDirection(glm::vec3(0.0f, -1.0f, -1.0f));
    tableNode->setMesh(table);
    tableNode->translate(glm::vec3(0.5f, 0.0f, -0.5f));
    tableNode->scale(glm::vec3(0.0035f));
    //node->setMesh(modelBackpack);
    node->translate(glm::vec3(0.0f, 0.0f, 0.0f));
    node->scale(glm::vec3(0.1f));
    suitcaseNode->setMesh(suitcase);
    suitcaseNode->translate(glm::vec3(-0.00f, 0.0f, -0.8f));
    suitcaseNode->scale(glm::vec3(0.0075f));
    suitcaseNode->rotate(FPI * 0.3f, glm::vec3(0.0f, 1.0f, 0.0f));
    chairNode->setMesh(chair);
    chairNode->scale(glm::vec3(0.5f));
    chairNode->translate(glm::vec3(-1.0f, 0.0f, 0.0f));
    planeNode->setMesh(plane);
    planeNode->translate(glm::vec3(0.0f, -0.0f, 0.0f));
    double timeDelta = 0.0;
    unsigned int secondCounter = 0U;
    auto guiProc = [&]()
    {
        ImGui::Begin("Settings");
        dirLight->drawGui();
        spotLight->drawGui();
        ImGui::End();
    };
    while (!appWindow.shouldClose())
    {
        appWindow.loop(timeDelta, guiProc);
        node->rotate(static_cast<float>(timeDelta) * 0.25f, glm::vec3(0.0f, 1.0f, 0.0f));
        resourceManager.updateShaders();
        double time = appWindow.getTime();
        if (static_cast<unsigned int>(time) > secondCounter)
        {
            secondCounter = static_cast<unsigned int>(time);
            SHADOW_INFO("{} FPS", appWindow.getFps());
        }
    }
    return 0;
}
