#define _CRT_SECURE_NO_WARNINGS
#include "AppWindow.h"
#include "ResourceManager.h"
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

struct BasicParams {
    unsigned int mapSize{};
};

struct PCFParams {
    unsigned int mapSize{};
    unsigned int filterSize{};
};

struct VSMParams {
    unsigned int mapSize{};
    unsigned int blurPasses{};
};

struct PCSSParams {
    unsigned int mapSize{};
    unsigned int shadowSamples{};
    unsigned int penumbraSamples{};
};

struct MasterCHSSParams {
    unsigned int mapSize{};
    unsigned int penumbraMapDivisor{};
    unsigned int shadowSamples{};
    unsigned int penumbraSamples{};
};

static const inline std::vector<unsigned int> MAP_SIZES = { 128, 256, 384, 512, 640, 768, 896, 1024, 1280, 1536, 1792, 2048, 2560, 3072, 3584, 4096 };
static const inline std::vector<unsigned int> FILTER_SIZES = { 1,3,5,7,9,11,15,19,23,27,31 };
static const inline std::vector<unsigned int> BLUR_PASSES = { 1,2,3,4,5 };
static const inline std::vector<unsigned int> PCSS_SHADOW_SAMPLES = { 4,6,8,12,14,16,20,24,28,32 };
static const inline std::vector<unsigned int> PCSS_PENUMBRA_SAMPLES = { 4,6,8,12,14,16,20,24,28,32 };
static const inline std::vector<unsigned int> SHADOW_SAMPLES = { 4,6,8,12,16,24,32,40,48,64 };
static const inline std::vector<unsigned int> PENUMBRA_SAMPLES = { 4,6,8,12,16,24,32,40,48,64 };
static const inline std::vector<unsigned int> PENUMBRA_MAP_DIVISORS = { 1,2,4,8,16 };

template<typename T>
std::vector<T> getAllParams() {
    throw std::runtime_error("Unhandled parameter type");
}

template<>
std::vector<BasicParams> getAllParams<BasicParams>()
{
    std::vector<BasicParams> result;
    for (unsigned int mapSize : MAP_SIZES) {
        result.push_back({ mapSize });
    }
    return result;
}

template<>
std::vector<PCFParams> getAllParams<PCFParams>()
{
    std::vector<PCFParams> result;
    for (unsigned int mapSize : MAP_SIZES) {
        for (unsigned int filterSize : FILTER_SIZES)
        {
            result.push_back({ mapSize, filterSize });
        }
    }
    return result;
}

template<>
std::vector<VSMParams> getAllParams<VSMParams>()
{
    std::vector<VSMParams> result;
    for (unsigned int mapSize : MAP_SIZES) {
        for (unsigned int blurPasses : BLUR_PASSES)
        {
            result.push_back({ mapSize, blurPasses });
        }
    }
    return result;
}

template<>
std::vector<PCSSParams> getAllParams<PCSSParams>()
{
    std::vector<PCSSParams> result;
    for (unsigned int mapSize : MAP_SIZES) {
        for (unsigned int shadowSamples : PCSS_SHADOW_SAMPLES)
        {
            for (unsigned int penumbraSamples : PCSS_PENUMBRA_SAMPLES)
            {
                result.push_back({ mapSize, shadowSamples, penumbraSamples });
            }
        }
    }
    return result;
}

template<>
std::vector<MasterCHSSParams> getAllParams<MasterCHSSParams>()
{
    std::vector<MasterCHSSParams> result;
    for (unsigned int mapSize : MAP_SIZES) {
        for (unsigned int penumbraMapDivisor : PENUMBRA_MAP_DIVISORS)
        {
            for (unsigned int shadowSamples : SHADOW_SAMPLES)
            {
                for (unsigned int penumbraSamples : PENUMBRA_SAMPLES)
                {
                    result.push_back({ mapSize, penumbraMapDivisor, shadowSamples, penumbraSamples });
                }
            }
        }
    }
    return result;
}

std::string getCommonCsvHeader() {
    return "Avg. FPS\tTotal frames\tBenchmark time [s]";
}

std::string formatCommonCsv(size_t frames, double benchmarkTime)
{
    return fmt::format("{}\t{}\t{}", frames / benchmarkTime, frames, benchmarkTime);
}

#if SHADOW_MASTER || SHADOW_CHSS
using ShadowParams = MasterCHSSParams;
void applyParameters(shadow::AppWindow& appWindow, shadow::ResourceManager& resourceManager, ShadowParams params) {
    appWindow.resizeLights(params.mapSize, params.penumbraMapDivisor);
    resourceManager.updateVogelDisk(params.shadowSamples, params.penumbraSamples);
}
#if SHADOW_MASTER
std::string getDirName() {
    return "Master";
}
#else
std::string getDirName() {
    return "CHSS";
}
#endif
std::string formatParams(const ShadowParams params) {
    return fmt::format("{}_{}_{}_{}_{}", getDirName(), params.mapSize, params.penumbraMapDivisor, params.shadowSamples, params.penumbraSamples);
}
std::string getCsvHeader() {
    return "Map size\tPenumbra texture size divisor\tShadow samples\tPenumbra samples";
}
std::string formatCsv(const ShadowParams params) {
    return fmt::format("{}\t{}\t{}\t{}", params.mapSize, params.penumbraMapDivisor, params.shadowSamples, params.penumbraSamples);
}
#elif SHADOW_PCSS
using ShadowParams = PCSSParams;
void applyParameters(shadow::AppWindow& appWindow, shadow::ResourceManager& resourceManager, ShadowParams params) {
    appWindow.resizeLights(params.mapSize);
    resourceManager.updatePoisson(params.shadowSamples, params.penumbraSamples);
}
std::string getDirName() {
    return "PCSS";
}
std::string formatParams(const ShadowParams params) {
    return fmt::format("{}_{}_{}_{}", getDirName(), params.mapSize, params.shadowSamples, params.penumbraSamples);
}
std::string getCsvHeader() {
    return "Map size\tShadow samples\tPenumbra samples";
}
std::string formatCsv(const ShadowParams params) {
    return fmt::format("{}\t{}\t{}", params.mapSize, params.shadowSamples, params.penumbraSamples);
}
#elif SHADOW_VSM
using ShadowParams = VSMParams;
void applyParameters(shadow::AppWindow& appWindow, shadow::ResourceManager& resourceManager, ShadowParams params) {
    appWindow.resizeLights(params.mapSize);
    appWindow.setBlurPasses(params.blurPasses);
}
std::string getDirName() {
    return "VSM";
}
std::string formatParams(const ShadowParams params) {
    return fmt::format("{}_{}_{}", getDirName(), params.mapSize, params.blurPasses);
}
std::string getCsvHeader() {
    return "Map size\tBlur passes";
}
std::string formatCsv(const ShadowParams params) {
    return fmt::format("{}\t{}", params.mapSize, params.blurPasses);
}
#elif SHADOW_PCF
using ShadowParams = PCFParams;
void applyParameters(shadow::AppWindow& appWindow, shadow::ResourceManager& resourceManager, ShadowParams params) {
    appWindow.resizeLights(params.mapSize);
    resourceManager.updateFilterSize(params.filterSize);
}
std::string getDirName() {
    return "PCF";
}
std::string formatParams(const ShadowParams params) {
    return fmt::format("{}_{}_{}", getDirName(), params.mapSize, params.filterSize);
}
std::string getCsvHeader() {
    return "Map size\tFilter size";
}
std::string formatCsv(const ShadowParams params) {
    return fmt::format("{}\t{}", params.mapSize, params.filterSize);
}
#else
using ShadowParams = BasicParams;
void applyParameters(shadow::AppWindow& appWindow, shadow::ResourceManager& resourceManager, ShadowParams params) {
    appWindow.resizeLights(params.mapSize);
}
std::string getDirName() {
    return "Basic";
}
std::string formatParams(const ShadowParams params) {
    return fmt::format("{}_{}", getDirName(), params.mapSize);
}
std::string getCsvHeader() {
    return "Map size";
}
std::string formatCsv(const ShadowParams params) {
    return fmt::format("{}", params.mapSize);
}
#endif

int main(int argc, char** argv)
{
    bool forceBenchmark = false;
    for (int i = 0; i < argc; ++i)
    {
        std::string arg = argv[i];
        if (arg == "benchmark") {
            forceBenchmark = true;
        }
    }
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
    dirLight->setColor(glm::vec3(0.0f, 0.0f, 1.0f));
    dirLight->setStrength(1.0f);
#if SHADOW_MASTER || SHADOW_CHSS || SHADOW_PCSS
    dirLight->setLightSize(0.09f);
#endif
    dirLight->setNearZ(0.2f);
    switch (SHADOW_IMPL) {
    default:
        dirLight->setFarZ(1.5f);
        break;
    case SHADOW_IMPL_BASIC:
    case SHADOW_IMPL_PCF:
        dirLight->setFarZ(8.0f);
        break;
    case SHADOW_IMPL_VSM:
        dirLight->setNearZ(0.3f);
        dirLight->setFarZ(2.0f);
        break;
    }
    dirLight->setProjectionSize(1.45f);
    dirLight->setPosition(glm::vec3(-0.03f, 1.0f, 0.4f));
    dirLight->setDirection(
        glm::quat(glm::vec3(glm::radians(-49.0f), glm::radians(15.0f), 0.0f))
        * glm::vec3(0.0f, 0.0f, -1.0f));
    spotLight->setColor(glm::vec3(1.0f, 0.0f, 0.0f));
    spotLight->setStrength(2.0f);
#if SHADOW_MASTER || SHADOW_CHSS || SHADOW_PCSS
    spotLight->setLightSize(0.09f);
#endif
    spotLight->setFarZ(2.35f);
    switch (SHADOW_IMPL) {
    default:
        spotLight->setNearZ(0.95f);
        break;
    case SHADOW_IMPL_BASIC:
    case SHADOW_IMPL_PCF:
        spotLight->setNearZ(0.2f);
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

    constexpr double BENCHMARK_TIME = 15.0f;
    double currentBenchmarkTime = 0.0;
    const std::vector<ShadowParams> benchmarkParams = getAllParams<ShadowParams>();
    size_t currentBenchmarkFrameCount = 0;
    unsigned int currentBenchmarkIndex = 0U;
    bool benchmarkRunning = false;
    bool benchmarkStarting = forceBenchmark;
    bool benchmarkWaitFrame = false;
    bool closeWindowAfterBenchmark = true;
    std::ostringstream benchmarkCsv;

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
        if (!benchmarkRunning) {
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
        if (benchmarkRunning)
        {
            if (!benchmarkWaitFrame)
            {
                ++currentBenchmarkFrameCount;
                currentBenchmarkTime += timeDelta;
                if (currentBenchmarkTime >= BENCHMARK_TIME)
                {
                    bool shadowsOnly = false;
#ifdef RENDER_SHADOW_ONLY
                    shadowsOnly = true;
#endif
                    const std::filesystem::path screenshotPath = std::filesystem::path(shadowsOnly ? getDirName() + "_Shadows" : getDirName()) / formatParams(benchmarkParams[currentBenchmarkIndex]);
                    appWindow.takeScreenshot(screenshotPath);
                    benchmarkCsv << getCsvHeader() << '\t' << getCommonCsvHeader() << std::endl;
                    benchmarkCsv << formatCsv(benchmarkParams[currentBenchmarkIndex]) << '\t' << formatCommonCsv(currentBenchmarkFrameCount, currentBenchmarkTime) << std::endl;
                    currentBenchmarkTime = 0.0f;
                    currentBenchmarkFrameCount = 0U;
                    ++currentBenchmarkIndex;
                    if (currentBenchmarkIndex < benchmarkParams.size())
                    {
                        benchmarkWaitFrame = true;
                        applyParameters(appWindow, resourceManager, benchmarkParams[currentBenchmarkIndex]);
                        if (resourceManager.reworkShaderFiles())
                        {
                            resourceManager.updateShaders();
                        }
                    }
                    else {
                        FILE* file = std::fopen((std::filesystem::path(shadowsOnly ? getDirName() + "_Shadows" : getDirName()) / (getDirName() + ".csv")).generic_string().c_str(), "w");
                        std::string csvString = benchmarkCsv.str();
                        benchmarkCsv.clear();
                        std::fwrite(csvString.c_str(), 1, csvString.length(), file);
                        std::fclose(file);
                        benchmarkRunning = false;
                        SHADOW_INFO("Benchmark finished!");
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
                benchmarkWaitFrame = true;
                applyParameters(appWindow, resourceManager, benchmarkParams[currentBenchmarkIndex]);
                if (resourceManager.reworkShaderFiles())
                {
                    resourceManager.updateShaders();
                }
                SHADOW_INFO("Beginning benchmark! Estimated time: {}s ({}s benchmark, {} parameter sets)", benchmarkParams.size() * BENCHMARK_TIME, BENCHMARK_TIME, benchmarkParams.size());
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
