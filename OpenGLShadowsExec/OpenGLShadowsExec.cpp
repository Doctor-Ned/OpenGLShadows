#include "AppWindow.h"
#include "ResourceManager.h"
#include "MaterialModelMesh.h"
#include "SceneNode.h"
#include "Primitives.h"
#include "ShadowUtils.h"

#include <glm/detail/type_quat.hpp>
#include <glm/ext/quaternion_trigonometric.hpp>
#include <glm/gtc/type_ptr.hpp>

#define GUI_UPDATE(value,oldValue,setter)     \
    do                                        \
    {                                         \
        if((oldValue) != (value))             \
        {                                     \
            setter(value);                    \
        }                                     \
    } while(false)

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

    bool analysing = false;
    float analysisMaxFps = 0.0f;
    float analysisAvgFps = 0.0f;
    float analysisTimer = 0.0f;
    unsigned int framesAnalysed = 0U;
    const float MIN_ANALYSIS_TIME = 10.0f;
    const unsigned int MIN_FRAMES_ANALYSED = 10000U;

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
    float maxFps = 0.0f;
    const int MAP_SIZE_COUNT = 16;
    GLsizei MAP_SIZES[MAP_SIZE_COUNT] = { 128, 256, 384, 512, 640, 768, 896, 1024, 1280, 1536, 1792, 2048, 2560, 3072, 3584, 4096 };
    int currMapSizeIndex = MAP_SIZE_COUNT - 1;
    int mapSize = MAP_SIZES[currMapSizeIndex];
    const int PENUMBRA_DIVISOR_COUNT = 9;
    unsigned int PENUMBRA_DIVISORS[PENUMBRA_DIVISOR_COUNT] = { 1,2,4,8,16,32,64,128,256 };
    int currPenumbraDivisorIndex = 0;
    unsigned int penumbraTextureSizeDivisor = PENUMBRA_DIVISORS[currPenumbraDivisorIndex];
    glm::vec3 spotPosition = spotData.position;
    appWindow.resizeLights(mapSize, penumbraTextureSizeDivisor);
    auto guiProc = [&]()
    {
        ImGui::Begin("Settings");
        ImGui::Text("FPS: %f", ImGui::GetIO().Framerate);
        maxFps = std::max(maxFps, ImGui::GetIO().Framerate);
        ImGui::Text("Max FPS: %f", maxFps);
        ImGui::SameLine();
        if (ImGui::Button("Reset"))
        {
            maxFps = 0.0f;
        }
        if (analysing)
        {
            if (ImGui::Button("Stop analysis"))
            {
                analysing = false;
            }
        }
        else
        {
            if (ImGui::Button("Run analysis"))
            {
                maxFps = 0.0f;
                analysisAvgFps = 0.0f;
                analysisMaxFps = 0.0f;
                analysisTimer = 0.0f;
                framesAnalysed = 0U;
                analysing = true;
            }
            ImGui::Checkbox("Show settings", &showingSettings);
            if (showingSettings)
            {
                ImGui::SliderInt("Shadow map size", &currMapSizeIndex, 0, MAP_SIZE_COUNT - 1, std::to_string(MAP_SIZES[currMapSizeIndex]).c_str());
                ImGui::SliderInt("Penumbra map size divisor", &currPenumbraDivisorIndex, 0, PENUMBRA_DIVISOR_COUNT - 1, std::to_string(PENUMBRA_DIVISORS[currPenumbraDivisorIndex]).c_str());
                ImGui::DragFloat("Directional light strength", &dirStrength, 0.05f, 0.0f, 25.0f);
                ImGui::DragFloat("Spot light strength", &spotStrength, 0.05f, 0.0f, 25.0f);
                ImGui::DragFloat("Directional light size", &dirSize, 0.005f, 0.0f, 5.0f);
                ImGui::DragFloat("Spot light size", &spotSize, 0.005f, 0.0f, 5.0f);
                ImGui::DragFloat3("Spot light position", value_ptr(spotPosition), 0.01f);
                ImGui::ColorPicker3("Directional light color", value_ptr(dirColor));
                ImGui::ColorPicker3("Spot light color", value_ptr(spotColor));
                ImGui::DragFloat("Dir projection size", &projectionSize, 0.05f, 0.0f, 15.0f);
                ImGui::DragFloat2("Directional clipping", value_ptr(dirClip), 0.05f, 0.0f, 10.0f);
                ImGui::DragFloat2("Spot clipping", value_ptr(spotClip), 0.05f, 0.0f, 10.0f);
                bool mapSizeChanged = false;
                if (mapSize != MAP_SIZES[currMapSizeIndex])
                {
                    mapSizeChanged = true;
                    mapSize = MAP_SIZES[currMapSizeIndex];
                }
                if (penumbraTextureSizeDivisor != PENUMBRA_DIVISORS[currPenumbraDivisorIndex])
                {
                    mapSizeChanged = true;
                    penumbraTextureSizeDivisor = PENUMBRA_DIVISORS[currPenumbraDivisorIndex];
                }
                if (mapSizeChanged)
                {
                    appWindow.resizeLights(mapSize, penumbraTextureSizeDivisor);
                }
                GUI_UPDATE(dirStrength, dirData.strength, dirLight->setStrength);
                GUI_UPDATE(spotStrength, spotData.strength, spotLight->setStrength);
                GUI_UPDATE(dirSize, dirData.lightSize, dirLight->setLightSize);
                GUI_UPDATE(spotSize, spotData.lightSize, spotLight->setLightSize);
                GUI_UPDATE(spotPosition, spotData.position, spotLight->setPosition);
                GUI_UPDATE(dirColor, dirData.color, dirLight->setColor);
                GUI_UPDATE(spotColor, spotData.color, spotLight->setColor);
                GUI_UPDATE(projectionSize, dirLight->getProjectionSize(), dirLight->setProjectionSize);
                GUI_UPDATE(dirClip.x, dirData.nearZ, dirLight->setNearZ);
                GUI_UPDATE(dirClip.y, dirData.farZ, dirLight->setFarZ);
                GUI_UPDATE(spotClip.x, spotData.nearZ, spotLight->setNearZ);
                GUI_UPDATE(spotClip.y, spotData.farZ, spotLight->setFarZ);
            }
        }
        ImGui::End();
    };
    while (!appWindow.shouldClose())
    {
        appWindow.loop(timeDelta, guiProc);
        if (analysing)
        {
            float fps = ImGui::GetIO().Framerate;
            analysisAvgFps += fps;
            analysisMaxFps = std::max(analysisMaxFps, fps);
            ++framesAnalysed;
            analysisTimer += static_cast<float>(timeDelta);
            if (analysisTimer >= MIN_ANALYSIS_TIME && framesAnalysed >= MIN_FRAMES_ANALYSED)
            {
                analysisAvgFps /= static_cast<float>(framesAnalysed);
                SHADOW_INFO("For map size {} -> FPS = [avg: {}, max: {}] ({} frames in {}s)", mapSize, analysisAvgFps, analysisMaxFps, framesAnalysed, analysisTimer); //todo: nicer output, maybe a file?
                analysing = false;
            }
        }
        else
        {
            if (resourceManager.reworkShaderFiles())
            {
                resourceManager.updateShaders();
            }
            double time = appWindow.getTime();
            if (static_cast<unsigned int>(time) > secondCounter)
            {
                secondCounter = static_cast<unsigned int>(time);
                SHADOW_INFO("{} FPS", ImGui::GetIO().Framerate);
            }
        }
    }
    return 0;
}
