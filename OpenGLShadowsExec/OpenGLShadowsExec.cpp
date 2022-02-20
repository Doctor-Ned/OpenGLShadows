#define _CRT_SECURE_NO_WARNINGS
#include "Benchmark.h"
#include "MaterialModelMesh.h"
#include "SceneNode.h"
#include "Primitives.h"
#include "ShadowUtils.h"
#include "ShadowVariants.h"

#include <glm/detail/type_quat.hpp>
#include <glm/ext/quaternion_trigonometric.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <fstream>
#include <sstream>

#define GUI_UPDATE(value,oldValue,setter)     \
    do                                        \
    {                                         \
        if((oldValue) != (value))             \
        {                                     \
            setter(value);                    \
        }                                     \
    } while(false)

int main(int argc, char** argv)
{
    using namespace shadow;
    bool forceBenchmark = false, genScreenshots = false;
    for (int i = 0; i < argc; ++i)
    {
        std::string arg = argv[i];
        if (arg == "benchmark") {
            forceBenchmark = true;
        }
        else if (arg == "screenshots") {
            genScreenshots = true;
        }
    }
    AppWindow& appWindow = AppWindow::getInstance();
    ResourceManager& resourceManager = ResourceManager::getInstance();
    if (!appWindow.initialize(1920, 1080, 1024, "../../Resources"))
    {
        return 1;
    }
    Configurator configurator(appWindow, resourceManager);
    std::shared_ptr<UboLights> uboLights = ResourceManager::getInstance().getUboLights();
    std::shared_ptr<DirectionalLight> dirLight = uboLights->getDirectionalLight();
    std::shared_ptr<SpotLight> spotLight = uboLights->getSpotLight();
    uboLights->setAmbient(0.1f);
#ifdef RENDER_SHADOW_ONLY
    dirLight->setColor(glm::vec3(0.0f, 0.0f, 1.0f));
    dirLight->setStrength(1.25f);
#else
    dirLight->setColor(glm::vec3(0.5f, 0.5f, 1.0f));
    dirLight->setStrength(5.0f);
#endif
#if SHADOW_MASTER || SHADOW_CHSS || SHADOW_PCSS
    dirLight->setLightSize(0.09f);
#endif
    dirLight->setProjectionSize(1.45f);
    dirLight->setNearZ(0.2f);
    switch (SHADOW_IMPL) {
    default:
        dirLight->setFarZ(1.5f);
        break;
    case SHADOW_IMPL_BASIC:
        dirLight->setFarZ(8.0f);
        break;
    case SHADOW_IMPL_PCF:
        dirLight->setProjectionSize(1.8f);
        dirLight->setNearZ(0.2f);
        dirLight->setFarZ(2.0f);
        break;
    case SHADOW_IMPL_VSM:
        dirLight->setNearZ(0.3f);
        dirLight->setFarZ(2.0f);
        break;
    }
    dirLight->setPosition(glm::vec3(-0.03f, 1.0f, 0.4f));
    dirLight->setDirection(
        glm::quat(glm::vec3(glm::radians(-49.0f), glm::radians(15.0f), 0.0f))
        * glm::vec3(0.0f, 0.0f, -1.0f));
#ifdef RENDER_SHADOW_ONLY
    spotLight->setColor(glm::vec3(1.0f, 0.0f, 0.0f));
    spotLight->setStrength(3.0f);
#else
    spotLight->setColor(glm::vec3(1.0f, 0.5f, 0.5f));
    spotLight->setStrength(15.0f);
#endif
#if SHADOW_MASTER || SHADOW_CHSS || SHADOW_PCSS
    spotLight->setLightSize(0.09f);
#endif
    spotLight->setFarZ(2.35f);
    switch (SHADOW_IMPL) {
    default:
        spotLight->setNearZ(0.95f);
        break;
    case SHADOW_IMPL_BASIC:
        spotLight->setNearZ(0.2f);
        break;
    case SHADOW_IMPL_PCF:
        spotLight->setNearZ(1.2f);
        spotLight->setFarZ(4.0f);
        break;
    case SHADOW_IMPL_VSM:
        spotLight->setNearZ(1.45f);
        spotLight->setFarZ(2.5f);
        break;
    }
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

    constexpr double BENCHMARK_TIME = 10.0f;
    double currentBenchmarkTime = 0.0;
    const std::vector<ShadowParams> benchmarkParams = configurator.getAllParams();
    size_t currentBenchmarkFrameCount = 0;
    unsigned int currentBenchmarkIndex = 0U;
    bool benchmarkRunning = false;
    bool benchmarkStarting = forceBenchmark;
    bool benchmarkWaitFrame = false;
    bool closeWindowAfterBenchmark = true;
    std::ostringstream benchmarkCsv;

    unsigned int currentScreenshotIndex = 0U;
    bool genScreenshotsRunning = false;
    bool genScreenshotsStarting = genScreenshots;
    bool genScreenshotsWaitFrame = true;
    bool closeWindowAfterGenScreenshots = genScreenshots && !forceBenchmark;


    double timeDelta = 0.0;
    unsigned int secondCounter = 0U;
    float maxFps = 0.0f;
    unsigned char screenshotState = 0;
    bool showingSettings = false;

    DirectionalLightData& dirData = dirLight->getData();
    SpotLightData& spotData = spotLight->getData();
    float dirStrength = dirData.strength, spotStrength = spotData.strength;
    glm::vec2 dirClip(dirData.nearZ, dirData.farZ), spotClip(spotData.nearZ, spotData.farZ);
    float dirSize = dirData.lightSize, spotSize = spotData.lightSize;
    float projectionSize = dirLight->getProjectionSize();
    glm::vec3 dirColor = dirData.color, spotColor = spotData.color;
    glm::vec3 spotPosition = spotData.position;

    const int MAP_SIZE_COUNT = 16;
    GLsizei MAP_SIZES[MAP_SIZE_COUNT] = { 128, 256, 384, 512, 640, 768, 896, 1024, 1280, 1536, 1792, 2048, 2560, 3072, 3584, 4096 };
    int currMapSizeIndex = MAP_SIZE_COUNT - 1;
    int mapSize = MAP_SIZES[currMapSizeIndex];

#if SHADOW_MASTER || SHADOW_CHSS || SHADOW_PCSS
    int currShadowSamples = 32, currPenumbraSamples = 16;
    unsigned int shadowSamples = currShadowSamples, penumbraSamples = currPenumbraSamples;
#endif
#if SHADOW_MASTER || SHADOW_CHSS
    const int PENUMBRA_DIVISOR_COUNT = 9;
    unsigned int PENUMBRA_DIVISORS[PENUMBRA_DIVISOR_COUNT] = { 1,2,4,8,16,32,64,128,256 };
    int currPenumbraDivisorIndex = 0;
    unsigned int penumbraTextureSizeDivisor = PENUMBRA_DIVISORS[currPenumbraDivisorIndex];
    resourceManager.updateVogelDisk(shadowSamples, penumbraSamples);
#elif SHADOW_PCSS
    resourceManager.updatePoisson(shadowSamples, penumbraSamples);
#elif SHADOW_PCF
    const int FILTER_SIZE_COUNT = 16;
    unsigned int FILTER_SIZES[FILTER_SIZE_COUNT] = { 1,3,5,7,9,11,13,15,17,19,21,23,25,27,29,31 };
    int currFilterSizeIndex = 0;
    unsigned int filterSize = FILTER_SIZES[currFilterSizeIndex];
    resourceManager.updateFilterSize(filterSize);
#elif SHADOW_VSM
    int blurPasses = appWindow.getBlurPasses();
#endif
#if SHADOW_MASTER || SHADOW_CHSS
    appWindow.resizeLights(mapSize, penumbraTextureSizeDivisor);
#else
    appWindow.resizeLights(mapSize);
#endif
    auto guiProc = [&]()
    {
        if (!genScreenshotsRunning && !benchmarkRunning) {
            if (screenshotState == 1) {
                ++screenshotState;
            }
            else {
                ImGui::Begin("Settings");
                ImGui::Text("FPS: %f", ImGui::GetIO().Framerate);
                maxFps = std::max(maxFps, ImGui::GetIO().Framerate);
                ImGui::Text("Max FPS: %f", maxFps);
                ImGui::SameLine();
                if (ImGui::Button("Reset"))
                {
                    maxFps = 0.0f;
                }
                if (ImGui::Button("Take screenshot"))
                {
                    screenshotState = 1;
                }
                if (ImGui::Button("Run benchmark"))
                {
                    benchmarkStarting = true;
                }
                ImGui::SameLine();
                ImGui::Checkbox("Close app after benchmark", &closeWindowAfterBenchmark);
                if (ImGui::Button("Generate screenshots"))
                {
                    genScreenshotsStarting = true;
                }
                ImGui::SameLine();
                ImGui::Checkbox("Close app after generating screenshots", &closeWindowAfterGenScreenshots);
                ImGui::Checkbox("Show settings", &showingSettings);
                if (showingSettings)
                {
                    ImGui::SliderInt("Shadow map size", &currMapSizeIndex, 0, MAP_SIZE_COUNT - 1, std::to_string(MAP_SIZES[currMapSizeIndex]).c_str());
#if SHADOW_MASTER || SHADOW_CHSS
                    ImGui::SliderInt("Penumbra map size divisor", &currPenumbraDivisorIndex, 0, PENUMBRA_DIVISOR_COUNT - 1, std::to_string(PENUMBRA_DIVISORS[currPenumbraDivisorIndex]).c_str());
#endif
#if SHADOW_MASTER || SHADOW_CHSS || SHADOW_PCSS
                    ImGui::SliderInt("Shadow samples", &currShadowSamples, 1, 64);
                    ImGui::SliderInt("Penumbra samples", &currPenumbraSamples, 1, 64);
#elif SHADOW_PCF
                    ImGui::SliderInt("Filter size", &currFilterSizeIndex, 0, FILTER_SIZE_COUNT - 1, std::to_string(FILTER_SIZES[currFilterSizeIndex]).c_str());
#elif SHADOW_VSM
                    ImGui::SliderInt("Blur passes", &blurPasses, 0, 100);
#endif
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
#if SHADOW_MASTER || SHADOW_CHSS
                    if (penumbraTextureSizeDivisor != PENUMBRA_DIVISORS[currPenumbraDivisorIndex])
                    {
                        mapSizeChanged = true;
                        penumbraTextureSizeDivisor = PENUMBRA_DIVISORS[currPenumbraDivisorIndex];
                    }
#endif
                    if (mapSizeChanged)
                    {
#if SHADOW_MASTER || SHADOW_CHSS
                        appWindow.resizeLights(mapSize, penumbraTextureSizeDivisor);
#else
                        appWindow.resizeLights(mapSize);
#endif
                    }
#if SHADOW_MASTER || SHADOW_CHSS || SHADOW_PCSS
                    if (shadowSamples != static_cast<unsigned int>(currShadowSamples) || penumbraSamples != static_cast<unsigned int>(currPenumbraSamples))
                    {
                        shadowSamples = currShadowSamples;
                        penumbraSamples = currPenumbraSamples;
#if SHADOW_MASTER || SHADOW_CHSS
                        resourceManager.updateVogelDisk(shadowSamples, penumbraSamples);
#else
                        resourceManager.updatePoisson(shadowSamples, penumbraSamples);
#endif
                    }
#elif SHADOW_PCF
                    if (filterSize != FILTER_SIZES[currFilterSizeIndex])
                    {
                        filterSize = FILTER_SIZES[currFilterSizeIndex];
                        resourceManager.updateFilterSize(filterSize);
                    }
#elif SHADOW_VSM
                    GUI_UPDATE(blurPasses, appWindow.getBlurPasses(), appWindow.setBlurPasses);
#endif
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
                ImGui::End();
            }
        }
    };
    while (!appWindow.shouldClose())
    {
        appWindow.loop(timeDelta, guiProc);
        if (genScreenshotsRunning)
        {
            if (!genScreenshotsWaitFrame)
            {
                const std::filesystem::path directory = configurator.getFullShadowName();
                appWindow.takeScreenshot(directory / configurator.formatParams(benchmarkParams[currentScreenshotIndex]));
                if (++currentScreenshotIndex < benchmarkParams.size())
                {
                    genScreenshotsWaitFrame = true;
                    configurator.applyParams(benchmarkParams[currentScreenshotIndex]);
                    if (resourceManager.reworkShaderFiles())
                    {
                        resourceManager.updateShaders();
                    }
                }
                else {
                    genScreenshotsRunning = false;
                    SHADOW_INFO("Finished generating screenshots!");
                    if (closeWindowAfterGenScreenshots) {
                        appWindow.close();
                    }
                }
            }
            else {
                genScreenshotsWaitFrame = false;
            }
        }
        else if (genScreenshotsStarting)
        {
            genScreenshotsRunning = true;
            genScreenshotsStarting = false;
            currentScreenshotIndex = 0U;
            genScreenshotsWaitFrame = true;
            SHADOW_INFO("Generating {} screenshots...", benchmarkParams.size());
            configurator.applyParams(benchmarkParams[currentScreenshotIndex]);
            if (resourceManager.reworkShaderFiles())
            {
                resourceManager.updateShaders();
            }
        }
        else if (benchmarkRunning)
        {
            if (!benchmarkWaitFrame)
            {
                ++currentBenchmarkFrameCount;
                currentBenchmarkTime += timeDelta;
                if (currentBenchmarkTime >= BENCHMARK_TIME)
                {
                    benchmarkCsv << configurator.formatCsv(benchmarkParams[currentBenchmarkIndex]) << '\t' << configurator.formatCommonCsv(currentBenchmarkFrameCount, currentBenchmarkTime) << std::endl;
                    SHADOW_INFO("[BM] {}% ({}/{}): {} -> {} ({} FPS)", (currentBenchmarkIndex + 1) * static_cast<size_t>(100) / benchmarkParams.size(), currentBenchmarkIndex + 1, benchmarkParams.size(), configurator.formatParams(benchmarkParams[currentBenchmarkIndex]), currentBenchmarkFrameCount, currentBenchmarkFrameCount / currentBenchmarkTime);
                    currentBenchmarkTime = 0.0f;
                    currentBenchmarkFrameCount = 0U;
                    ++currentBenchmarkIndex;
                    if (currentBenchmarkIndex < benchmarkParams.size())
                    {
                        benchmarkWaitFrame = true;
                        configurator.applyParams(benchmarkParams[currentBenchmarkIndex]);
                        if (resourceManager.reworkShaderFiles())
                        {
                            resourceManager.updateShaders();
                        }
                    }
                    else {
                        const std::filesystem::path csvFile = (std::filesystem::path(configurator.getFullShadowName()) / (configurator.getShadowName() + ".csv"));
                        FILE* file = std::fopen(csvFile.generic_string().c_str(), "w");
                        std::string csvString = benchmarkCsv.str();
                        benchmarkCsv.clear();
                        std::fwrite(csvString.c_str(), 1, csvString.length(), file);
                        std::fclose(file);
                        benchmarkRunning = false;
                        SHADOW_INFO("[BM] Benchmark finished! CSV: '{}'", csvFile.generic_string());
                        if (closeWindowAfterBenchmark)
                        {
                            appWindow.close();
                        }
                    }
                }
            }
            else {
                benchmarkWaitFrame = false;
            }
        }
        else {
            if (benchmarkStarting)
            {
                benchmarkRunning = true;
                benchmarkStarting = false;
                currentBenchmarkTime = 0.0f;
                currentBenchmarkIndex = 0U;
                currentBenchmarkFrameCount = 0U;
                benchmarkCsv.clear();
                benchmarkCsv << configurator.getCsvHeader() << '\t' << configurator.getCommonCsvHeader() << std::endl;
                benchmarkWaitFrame = true;
                configurator.applyParams(benchmarkParams[currentBenchmarkIndex]);
                if (resourceManager.reworkShaderFiles())
                {
                    resourceManager.updateShaders();
                }
                int estimatedSeconds = static_cast<int>(benchmarkParams.size() * BENCHMARK_TIME + 0.5f);
                int estimatedMinutes = estimatedSeconds / 60;
                estimatedSeconds %= 60;
                int estimatedHours = estimatedMinutes / 60;
                estimatedMinutes %= 60;
                SHADOW_INFO("[BM] Beginning benchmark! Estimated time: [{}h:{}m:{}s] ({}s benchmark, {} parameter sets)", estimatedHours, estimatedMinutes, estimatedSeconds, BENCHMARK_TIME, benchmarkParams.size());
            }
            else {
                if (screenshotState == 2)
                {
                    screenshotState = 0;
                    appWindow.takeScreenshot("screenshot");
                }
                if (resourceManager.reworkShaderFiles())
                {
                    resourceManager.updateShaders();
                }
            }
        }
    }
    return 0;
}
