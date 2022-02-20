#pragma once
#include "AppWindow.h"

static const inline std::vector<unsigned int> MAP_SIZES = { 256, 512, 768, 1024, 1280, 1536, 1792, 2048, 2560, 3072, 3584, 4096 };
static const inline std::vector<unsigned int> FILTER_SIZES = { 1,3,5,7,9,11,15,19,23,27,31 };
static const inline std::vector<unsigned int> BLUR_PASSES = { 1,2,3,4,5 };
static const inline std::vector<unsigned int> SHADOW_SAMPLES = { 4,8,12,16,32 };
static const inline std::vector<unsigned int> PENUMBRA_SAMPLES = { 8,16,24,32 };
static const inline std::vector<unsigned int> PENUMBRA_MAP_DIVISORS = { 1,2,4,8 };

namespace shadow {
    template<typename Params>
    class ShadowConfigurator {
    public:
        ShadowConfigurator(AppWindow& appWindow, ResourceManager& resourceManager) : appWindow(appWindow), resourceManager(resourceManager) {}
        virtual ~ShadowConfigurator() = default;
        virtual void applyParams(const Params& params) = 0;
        std::string getFullShadowName() const {
#ifdef RENDER_SHADOW_ONLY
            return getShadowName() + "_Shadows";
#else
            return getShadowName();
#endif
        }
        virtual std::string getShadowName() const = 0;
        virtual std::string getCsvHeader() const = 0;
        virtual std::string formatCsv(const Params& params) const = 0;
        virtual std::string formatParams(const Params& params) const = 0;
        virtual std::vector<Params> getAllParams() const = 0;
        virtual std::map<unsigned int, Params> getBestParams() const = 0;
        static std::string getCommonCsvHeader() {
            return "Avg. FPS\tTotal frames\tBenchmark time [s]";
        }
        static std::string formatCommonCsv(size_t frames, double benchmarkTime)
        {
            return fmt::format("{}\t{}\t{}", frames / benchmarkTime, frames, benchmarkTime);
        }
    protected:
        AppWindow& appWindow;
        ResourceManager& resourceManager;
    };

#if SHADOW_MASTER || SHADOW_CHSS
    struct MasterCHSSParams {
        unsigned int mapSize{};
        unsigned int penumbraMapDivisor{};
        unsigned int shadowSamples{};
        unsigned int penumbraSamples{};
    };
    using ShadowParams = MasterCHSSParams;
    class Configurator : public ShadowConfigurator<ShadowParams> {
    public:
        Configurator(AppWindow& appWindow, ResourceManager& resourceManager) : ShadowConfigurator(appWindow, resourceManager) {}
        void applyParams(const ShadowParams& params) override {
            appWindow.resizeLights(params.mapSize, params.penumbraMapDivisor);
            resourceManager.updateVogelDisk(params.shadowSamples, params.penumbraSamples);
        }
        std::string getShadowName() const override {
#if SHADOW_MASTER
            return "Master";
#else
            return "CHSS";
#endif
        }
        std::string getCsvHeader() const override {
            return "Map size\tPenumbra texture size divisor\tShadow samples\tPenumbra samples";
        }
        std::string formatCsv(const ShadowParams& params) const override {
            return fmt::format("{}\t{}\t{}\t{}", params.mapSize, params.penumbraMapDivisor, params.shadowSamples, params.penumbraSamples);
        }
        std::string formatParams(const ShadowParams& params) const override {
            return fmt::format("{}_{}_{}_{}_{}", getShadowName(), params.mapSize, params.penumbraMapDivisor, params.shadowSamples, params.penumbraSamples);
        }
        std::vector<ShadowParams> getAllParams() const override {
            std::vector<ShadowParams> result;
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
        std::map<unsigned int, ShadowParams> getBestParams() const override {
            std::map<unsigned int, ShadowParams> result{
#if SHADOW_MASTER
                {1200, {896,2,14,14}},
                {800, {2048,4,18,18}},
                {400, {3776,4,20,20}}
#else
                {1200, {832,2,12,14}},
                {800, {2048,4,16,16}},
                {400, {3776,4,20,20}}
#endif
            };
            return result;
        }
    };
#elif SHADOW_PCSS
    struct PCSSParams {
        unsigned int mapSize{};
        unsigned int shadowSamples{};
        unsigned int penumbraSamples{};
    };
    using ShadowParams = PCSSParams;
    class Configurator : public ShadowConfigurator<ShadowParams> {
    public:
        Configurator(AppWindow& appWindow, ResourceManager& resourceManager) : ShadowConfigurator(appWindow, resourceManager) {}
        void applyParams(const ShadowParams& params) override {
            appWindow.resizeLights(params.mapSize);
            resourceManager.updatePoisson(params.shadowSamples, params.penumbraSamples);
        }
        std::string getShadowName() const override {
            return "PCSS";
        }
        std::string getCsvHeader() const override {
            return "Map size\tShadow samples\tPenumbra samples";
        }
        std::string formatCsv(const ShadowParams& params) const override {
            return fmt::format("{}\t{}\t{}", params.mapSize, params.shadowSamples, params.penumbraSamples);
        }
        std::string formatParams(const ShadowParams& params) const override {
            return fmt::format("{}_{}_{}_{}", getShadowName(), params.mapSize, params.shadowSamples, params.penumbraSamples);
        }
        std::vector<ShadowParams> getAllParams() const override {
            std::vector<ShadowParams> result;
            for (unsigned int mapSize : MAP_SIZES) {
                for (unsigned int shadowSamples : SHADOW_SAMPLES)
                {
                    for (unsigned int penumbraSamples : PENUMBRA_SAMPLES)
                    {
                        result.push_back({ mapSize, shadowSamples, penumbraSamples });
                    }
                }
            }
            return result;
        }
        std::map<unsigned int, ShadowParams> getBestParams() const override {
            std::map<unsigned int, ShadowParams> result{
                {1200, {768,12,12}},
                {800, {1728,16,16}},
                {400, {3072,20,20}}
            };
            return result;
        }
    };
#elif SHADOW_VSM
    struct VSMParams {
        unsigned int mapSize{};
        unsigned int blurPasses{};
    };
    using ShadowParams = VSMParams;
    class Configurator : public ShadowConfigurator<ShadowParams> {
    public:
        Configurator(AppWindow& appWindow, ResourceManager& resourceManager) : ShadowConfigurator(appWindow, resourceManager) {}
        void applyParams(const ShadowParams& params) override {
            appWindow.resizeLights(params.mapSize);
            appWindow.setBlurPasses(params.blurPasses);
        }
        std::string getShadowName() const override {
            return "VSM";
        }
        std::string getCsvHeader() const override {
            return "Map size\tBlur passes";
        }
        std::string formatCsv(const ShadowParams& params) const override {
            return fmt::format("{}\t{}", params.mapSize, params.blurPasses);
        }
        std::string formatParams(const ShadowParams& params) const override {
            return fmt::format("{}_{}_{}", getShadowName(), params.mapSize, params.blurPasses);
        }
        std::vector<ShadowParams> getAllParams() const override {
            std::vector<ShadowParams> result;
            for (unsigned int mapSize : MAP_SIZES) {
                for (unsigned int blurPasses : BLUR_PASSES)
                {
                    result.push_back({ mapSize, blurPasses });
                }
            }
            return result;
        }
        std::map<unsigned int, ShadowParams> getBestParams() const override {
            std::map<unsigned int, ShadowParams> result{
                {1200, {1728,1}},
                {800, {2496,1}},
                {400, {4032,1}}
            };
            return result;
        }
    };
#elif SHADOW_PCF
    struct PCFParams {
        unsigned int mapSize{};
        unsigned int filterSize{};
    };
    using ShadowParams = PCFParams;
    class Configurator : public ShadowConfigurator<ShadowParams> {
    public:
        Configurator(AppWindow& appWindow, ResourceManager& resourceManager) : ShadowConfigurator(appWindow, resourceManager) {}
        void applyParams(const ShadowParams& params) override {
            appWindow.resizeLights(params.mapSize);
            resourceManager.updateFilterSize(params.filterSize);
        }
        std::string getShadowName() const override {
            return "PCF";
        }
        std::string getCsvHeader() const override {
            return "Map size\tFilter size";
        }
        std::string formatCsv(const ShadowParams& params) const override {
            return fmt::format("{}\t{}", params.mapSize, params.filterSize);
        }
        std::string formatParams(const ShadowParams& params) const override {
            return fmt::format("{}_{}_{}", getShadowName(), params.mapSize, params.filterSize);
        }
        std::vector<ShadowParams> getAllParams() const override {
            std::vector<ShadowParams> result;
            for (unsigned int mapSize : MAP_SIZES) {
                for (unsigned int filterSize : FILTER_SIZES)
                {
                    result.push_back({ mapSize, filterSize });
                }
            }
            return result;
        }
        std::map<unsigned int, ShadowParams> getBestParams() const override {
            std::map<unsigned int, ShadowParams> result{
                {1200, {1728,3}},
                {800, {2112,5}},
                {400, {2496,9}}
            };
            return result;
        }
    };
#else
    struct BasicParams {
        unsigned int mapSize{};
    };
    using ShadowParams = BasicParams;
    class Configurator : public ShadowConfigurator<ShadowParams> {
    public:
        Configurator(AppWindow& appWindow, ResourceManager& resourceManager) : ShadowConfigurator(appWindow, resourceManager) {}
        void applyParams(const ShadowParams& params) override {
            appWindow.resizeLights(params.mapSize);
        }
        std::string getShadowName() const override {
            return "Basic";
        }
        std::string getCsvHeader() const override {
            return "Map size";
        }
        std::string formatCsv(const ShadowParams& params) const override {
            return fmt::format("{}", params.mapSize);
        }
        std::string formatParams(const ShadowParams& params) const override {
            return fmt::format("{}_{}", getShadowName(), params.mapSize);
        }
        std::vector<ShadowParams> getAllParams() const override {
            std::vector<ShadowParams> result;
            for (unsigned int mapSize : MAP_SIZES) {
                result.push_back({ mapSize });
            }
            return result;
        }
        std::map<unsigned int, ShadowParams> getBestParams() const override {
            std::map<unsigned int, ShadowParams> result{
                {1200, {1984}},
                {800, {2880}},
                {400, {4608}}
            };
            return result;
        }
    };
#endif
}