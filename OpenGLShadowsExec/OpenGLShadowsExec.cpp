#include "AppWindow.h"
#include "ResourceManager.h"
#include "MaterialModelMesh.h"
#include "SceneNode.h"
#include "Primitives.h"
#include "ShadowUtils.h"

#include <glm/detail/type_quat.hpp>
#include <glm/ext/quaternion_trigonometric.hpp>
#include <glm/gtc/type_ptr.hpp>

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
    dirLight->setColor(glm::vec3(1.0f, 1.0f, 1.0f));
    dirLight->setStrength(1.8f);
    dirLight->setLightSize(0.09f);
    dirLight->setNearZ(0.2f);
    dirLight->setFarZ(1.5f);
    dirLight->setProjectionSize(1.45f);
    dirLight->setPosition(glm::vec3(-0.03f, 1.0f, 0.4f));
    dirLight->setDirection(
        glm::quat(glm::vec3(glm::radians(-49.0f), glm::radians(15.0f), 0.0f))
        * glm::vec3(0.0f, 0.0f, -1.0f));
    spotLight->setColor(glm::vec3(1.0f, 0.7f, 0.28f));
    spotLight->setStrength(6.8f);
    spotLight->setLightSize(0.09f);
    spotLight->setNearZ(0.95f);
    spotLight->setFarZ(2.35f);
    spotLight->setInnerCutOff(cosf(glm::radians(20.0f)));
    spotLight->setOuterCutOff(cosf(glm::radians(25.0f)));
    spotLight->setPosition(glm::vec3(1.07f, 1.6f, 0.4f));
    spotLight->setDirection(
        glm::quat(glm::vec3(glm::radians(-58.0f), glm::radians(67.0f), 0.0f))
        * glm::vec3(0.0f, 0.0f, -1.0f));
    std::shared_ptr<Camera> camera = appWindow.getCamera();
    camera->setPosition(glm::vec3(-0.78f, 1.02f, 0.45f));
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
    tableNode->setMesh(table).translate(glm::vec3(-0.03f, 0.0f, -0.1f)).scale(glm::vec3(0.0035f));
    suitcaseNode->setMesh(suitcase).scale(glm::vec3(0.0055f)).rotate(-FPI * 0.3f, glm::vec3(0.0f, 1.0f, 0.0f)).setPosition(glm::vec3(0.07f, 0.267f, -0.2f));
    chairNode->setMesh(chair).scale(glm::vec3(0.5f)).rotate(FPI * 0.85f, glm::vec3(0.0f, 1.0f, 0.0f)).setPosition(glm::vec3(-0.03f, 0.0f, 0.3f));
    planeNode->setMesh(plane).translate(glm::vec3(0.0f, -0.0f, 0.0f));
    scene->setParent(node, tableNode);
    scene->setParent(node, suitcaseNode);
    scene->setParent(node, chairNode);
    scene->setParent(node, planeNode);
    double timeDelta = 0.0;
    unsigned int secondCounter = 0U;

    bool showingSettings = false;
    DirectionalLightData& dirData = dirLight->getData();
    SpotLightData& spotData = spotLight->getData();
    float dirStrength = dirData.strength, spotStrength = spotData.strength;
    glm::vec2 dirClip(dirData.nearZ, dirData.farZ), spotClip(spotData.nearZ, spotData.farZ);
    float dirSize = dirData.lightSize, spotSize = spotData.lightSize;
    float projectionSize = dirLight->getProjectionSize();
    glm::vec3 dirColor = dirData.color, spotColor = spotData.color;
    enum ShadowMapSize { Small256, Medium512, Big1024, Huge2048, Enormous4096 };
    int currentMapSize = Medium512;

    const char* MAP_SIZES[] = { "Small(256)", "Medium(512)", "Big(1024)", "Huge(2048)", "Enormous(4096)" };
    auto guiProc = [&]()
    {
        ImGui::Begin("Settings");
        ImGui::Checkbox("Show settings", &showingSettings);
        if (showingSettings)
        {
            int mapSize = currentMapSize;
            const char* MAP_SIZE_NAME = MAP_SIZES[currentMapSize];
            ImGui::SliderInt("Shadow map size", &mapSize, 0, 3, MAP_SIZE_NAME);
            ImGui::DragFloat("Directional light strength", &dirStrength, 0.05f, 0.0f, 25.0f);
            ImGui::DragFloat("Spot light strength", &spotStrength, 0.05f, 0.0f, 25.0f);
            ImGui::DragFloat("Directional light size", &dirSize, 0.05f, 0.0f, 10.0f);
            ImGui::DragFloat("Spot light size", &spotSize, 0.05f, 0.0f, 10.0f);
            ImGui::ColorPicker3("Directional light color", value_ptr(dirColor));
            ImGui::ColorPicker3("Spot light color", value_ptr(spotColor));
            ImGui::DragFloat("Dir projection size", &projectionSize, 0.05f, 0.0f, 15.0f);
            ImGui::DragFloat2("Directional clipping", value_ptr(dirClip), 0.05f, 0.0f, 10.0f);
            ImGui::DragFloat2("Spot clipping", value_ptr(spotClip), 0.05f, 0.0f, 10.0f);
            if (currentMapSize != mapSize)
            {
                currentMapSize = mapSize;
                switch (currentMapSize)
                {
                    case Small256:
                        appWindow.resizeLights(256);
                        break;
                    case Medium512:
                        appWindow.resizeLights(512);
                        break;
                    case Big1024:
                        appWindow.resizeLights(1024);
                        break;
                    case Huge2048:
                        appWindow.resizeLights(2048);
                        break;
                    case Enormous4096:
                        appWindow.resizeLights(4096);
                        break;
                }
            }
            if (dirData.strength != dirStrength)
            {
                dirLight->setStrength(dirStrength);
            }
            if (spotData.strength != spotStrength)
            {
                spotLight->setStrength(spotStrength);
            }
            if (dirData.lightSize != dirSize)
            {
                dirLight->setLightSize(dirSize);
            }
            if (spotData.lightSize != spotSize)
            {
                spotLight->setLightSize(spotSize);
            }
            if (dirData.color != dirColor)
            {
                dirLight->setColor(dirColor);
            }
            if (spotData.color != spotColor)
            {
                spotLight->setColor(spotColor);
            }
            if (dirLight->getProjectionSize() != projectionSize)
            {
                dirLight->setProjectionSize(projectionSize);
            }
            if (dirData.nearZ != dirClip.x)
            {
                dirLight->setNearZ(dirClip.x);
            }
            if (dirData.farZ != dirClip.y)
            {
                dirLight->setFarZ(dirClip.y);
            }
            if (spotData.nearZ != spotClip.x)
            {
                spotLight->setNearZ(spotClip.x);
            }
            if (spotData.farZ != spotClip.y)
            {
                spotLight->setFarZ(spotClip.y);
            }
        }
        ImGui::End();
    };
    while (!appWindow.shouldClose())
    {
        appWindow.loop(timeDelta, guiProc);
        node->rotate(static_cast<float>(timeDelta) * 0.25f, glm::vec3(0.0f, 1.0f, 0.0f));
        if (resourceManager.reworkShaderFiles())
        {
            resourceManager.updateShaders();
        }
        double time = appWindow.getTime();
        if (static_cast<unsigned int>(time) > secondCounter)
        {
            secondCounter = static_cast<unsigned int>(time);
            SHADOW_INFO("{} FPS", appWindow.getFps());
        }
    }
    return 0;
}
