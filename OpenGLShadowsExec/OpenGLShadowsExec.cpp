#include "AppWindow.h"
#include "ResourceManager.h"
#include "MaterialModelMesh.h"
#include "SceneNode.h"
#include "Primitives.h"
#include "ShadowUtils.h"

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/detail/type_quat.hpp>
#include <glm/ext/quaternion_trigonometric.hpp>

int main()
{
    using namespace shadow;
    AppWindow& appWindow = AppWindow::getInstance();
    ResourceManager& resourceManager = ResourceManager::getInstance();
    if (!appWindow.initialize(1920, 1080, 1024, "../../Resources"))
    {
        return 1;
    }
    std::shared_ptr<UboLights> uboLights = ResourceManager::getInstance().getUboLights();
    std::shared_ptr<DirectionalLight> dirLight = uboLights->getDirectionalLight();
    std::shared_ptr<SpotLight> spotLight = uboLights->getSpotLight();
    uboLights->setAmbient(0.1f);
    dirLight->setColor(glm::vec3(0.5f, 1.0f, 1.0f));
    dirLight->setStrength(3.5f);
    dirLight->setNearZ(0.1f);
    dirLight->setFarZ(1.5f);
    dirLight->setProjectionSize(1.5f);
    dirLight->setPosition(glm::vec3(0.5f, 1.0f, 0.0f));
    dirLight->setDirection(
        glm::quat(glm::vec3(glm::radians(-49.0f), glm::radians(15.0f), 0.0f))
        * glm::vec3(0.0f, 0.0f, -1.0f));
    spotLight->setColor(glm::vec3(1.0f, 0.5f, 1.0f));
    spotLight->setStrength(5.5f);
    spotLight->setNearZ(0.1f);
    spotLight->setFarZ(4.5f);
    spotLight->setInnerCutOff(cosf(glm::radians(20.0f)));
    spotLight->setOuterCutOff(cosf(glm::radians(25.0f)));
    spotLight->setPosition(glm::vec3(1.6f, 1.6f, 0.0f));
    spotLight->setDirection(
        glm::quat(glm::vec3(glm::radians(-58.0f), glm::radians(67.0f), 0.0f))
        * glm::vec3(0.0f, 0.0f, -1.0f));
    std::shared_ptr<Camera> camera = appWindow.getCamera();
    camera->setPosition(glm::vec3(-0.25f, 1.02f, 0.05f));
    camera->setDirection(
        glm::quat(glm::vec3(glm::radians(-52.0f), glm::radians(-46.0f), 0.0f))
        * glm::vec3(0.0f, 0.0f, -1.0f));
    std::shared_ptr<PrimitiveData> planeData = Primitives::plane(5.0f, 5.0f, glm::vec2(5.0f));
    std::shared_ptr<TextureMesh> plane = std::make_shared<TextureMesh>(
        planeData->toTextureVertex(), planeData->getIndices(),
        std::map <TextureType, std::shared_ptr<Texture>> {
            { TextureType::Albedo, resourceManager.getTexture("Planks/planks_albedo.png") },
            { TextureType::Roughness, resourceManager.getTexture("Planks/planks_roughness.png") },
            { TextureType::Metalness, resourceManager.getTexture("Planks/planks_metallic.png") },
            { TextureType::Normal, resourceManager.getTexture("Planks/planks_normal.png") }
    });
    std::shared_ptr<ModelMesh> table = resourceManager.getModel("Table/Table.obj");
    std::shared_ptr<ModelMesh> suitcase = resourceManager.getModel("Suitcase/Vintage_Suitcase_LP.obj");
    std::shared_ptr<ModelMesh> chair = resourceManager.getModel("Chair/Chair.obj");
    std::shared_ptr<Scene> scene = appWindow.getScene();
    std::shared_ptr<SceneNode> node = scene->addNode(), suitcaseNode = scene->addNode(), chairNode = scene->addNode(), planeNode = scene->addNode(), tableNode = scene->addNode();
    tableNode->setMesh(table);
    tableNode->translate(glm::vec3(0.5f, 0.0f, -0.5f));
    tableNode->scale(glm::vec3(0.0035f));
    //node->setMesh(modelBackpack);
    suitcaseNode->setMesh(suitcase);
    suitcaseNode->scale(glm::vec3(0.0055f));
    suitcaseNode->rotate(-FPI * 0.3f, glm::vec3(0.0f, 1.0f, 0.0f));
    suitcaseNode->setPosition(glm::vec3(0.6f, 0.267f, -0.6f));
    chairNode->setMesh(chair);
    chairNode->scale(glm::vec3(0.5f));
    chairNode->rotate(FPI * 0.85f, glm::vec3(0.0f, 1.0f, 0.0f));
    chairNode->setPosition(glm::vec3(0.5f, 0.0f, -0.1f));
    planeNode->setMesh(plane);
    planeNode->translate(glm::vec3(0.0f, -0.0f, 0.0f));
    double timeDelta = 0.0;
    unsigned int secondCounter = 0U;
    auto guiProc = [&]()
    {
        ImGui::Begin("Settings");
        //dirLight->drawGui();
        //spotLight->drawGui();
        //static glm::vec3 pos = chairNode->getModel()[3];
        //ImGui::DragFloat3("Chair pos", &pos[0], 0.1f);
        //glm::mat4 mod = chairNode->getModel();
        //mod[3] = glm::vec4(pos, 1.0f);
        //chairNode->setModel(mod);
        //static float camX{}, camY{}, camZ{};
        //static glm::vec3 position = camera->getPosition();
        //ImGui::SliderAngle("CamX", &camX);
        //ImGui::SliderAngle("CamY", &camY);
        //ImGui::SliderAngle("CamZ", &camZ);
        //camera->setDirection(glm::quat(glm::vec3(camX, camY, camZ)) * glm::vec3(0.0f, 0.0f, -1.0f));
        //ImGui::DragFloat3("CamPos", &position[0], 0.25f);
        //camera->setPosition(position);
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
