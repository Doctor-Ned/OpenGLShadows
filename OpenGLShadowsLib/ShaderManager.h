#pragma once

#include "ShadowLog.h"

#include "ShaderType.h"
#include "UboMvp.h"
#include "UboMaterial.h"
#include "UboLights.h"
#include "UboWindow.h"
#include "ShadowVariants.h"

#include <map>
#include <set>

namespace shadow
{
    struct ShaderFileInfo final
    {
        std::set<std::filesystem::path> references{}; // external file includes
        std::set<std::string> includes{}; // plain-text includes provided by this class
        std::filesystem::file_time_type timestamp{};
        std::string content{}; // file content (contains includes)
        bool modified{};
    };

    struct ShaderTextInclude final
    {
        std::string content{};
        bool modified{};
    };

    class ShaderManager
    {
    public:
        ~ShaderManager() = default;
        ShaderManager(ShaderManager&) = delete;
        ShaderManager(ShaderManager&&) = delete;
        ShaderManager& operator=(ShaderManager&) = delete;
        ShaderManager& operator=(ShaderManager&&) = delete;
        bool reworkShaderFiles();
        void updateShaders() const;
#if SHADOW_MASTER || SHADOW_CHSS
        void updateVogelDisk(unsigned int shadowSamples, unsigned int penumbraSamples);
#elif SHADOW_PCSS
        void updatePoisson(unsigned int shadowSamples, unsigned int penumbraSamples);
#elif SHADOW_PCF
        void updateFilterSize(unsigned int filterSize);
#endif
        std::string getShaderFileContent(const std::filesystem::path& path);
        std::shared_ptr<GLShader> getShader(ShaderType shaderType);
        std::shared_ptr<UboMvp> getUboMvp() const;
        std::shared_ptr<UboMaterial> getUboMaterial() const;
        std::shared_ptr<UboLights> getUboLights() const;
        std::shared_ptr<UboWindow> getUboWindow() const;
    private:
        friend class ResourceManager;
        ShaderManager(const std::filesystem::path& shadersDirectory);
        bool rebuildShaderFile(const std::filesystem::path& path);
        bool isShaderFileRecursivelyReferenced(const std::filesystem::path& path, const std::filesystem::path& searchPath);
        bool isShaderFileModified(const std::filesystem::path& path);
        void loadShaders(GLsizei windowWidth, GLsizei windowHeight);
        void updateInclude(const std::string& inclName, const std::string& inclContent);
        void prepareShaderIncludes(GLsizei windowWidth, GLsizei windowHeight);
        void addShaderInclude(const std::string& name, const std::string& content);
        std::string getShaderImplIncludeContent() const;
#if SHADOW_MASTER || SHADOW_CHSS
        std::string getVogelIncludeContent(unsigned int shadowSamples, unsigned int penumbraSamples) const;
        std::vector<glm::vec2> getVogelDisk(unsigned int size) const;
#elif SHADOW_PCSS
        std::string getPoissonIncludeContent(unsigned int shadowSamples, unsigned int penumbraSamples) const;
#elif SHADOW_PCF
        std::string getFilterSizeIncludeContent(unsigned int filterSize) const;
#endif
        std::map<std::filesystem::path, ShaderFileInfo> shaderFileInfos{};
        std::map<ShaderType, std::shared_ptr<GLShader>> shaders{};
        std::map<std::string, ShaderTextInclude> shaderIncludes{};
        std::shared_ptr<UboMvp> uboMvp{};
        std::shared_ptr<UboMaterial> uboMaterial{};
        std::shared_ptr<UboLights> uboLights{};
        std::shared_ptr<UboWindow> uboWindow{};
        const char* INCLUDE_TEXT = "//SHADOW>include ", * INCLUDED_FROM_TEXT = "//SHADOW>includedfrom ", * END_INCLUDE_TEXT = "//SHADOW>endinclude ", * REFILL_TEXT = "//SHADOW>refill";
        const std::string SHADOW_IMPL_INCLUDE_TEXT{ "SHADOW_IMPL" };
#if SHADOW_MASTER || SHADOW_CHSS
        const std::string VOGEL_INCLUDE_TEXT{ "VOGEL_DISK" };
#elif SHADOW_PCSS
        const std::string POISSON_INCLUDE_TEXT{ "POISSON" };
#elif SHADOW_PCF
        const std::string FILTER_SIZE_INCLUDE_TEXT{ "FILTER_SIZE" };
#endif
        const size_t INCLUDE_LENGTH = strlen(INCLUDE_TEXT), INCLUDED_FROM_LENGTH = strlen(INCLUDED_FROM_TEXT), END_INCLUDE_LENGTH = strlen(END_INCLUDE_TEXT), REFILL_LENGTH = strlen(REFILL_TEXT);
        const std::vector<std::string> SHADER_EXTENSIONS{ ".glsl", ".vert", ".frag" };
        std::filesystem::path shadersDirectory{};
    };
}
