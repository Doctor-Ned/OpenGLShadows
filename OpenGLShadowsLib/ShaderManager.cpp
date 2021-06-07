#include "ShaderManager.h"
#include "GLShader.h"
#include "ShadowUtils.h"

#include <fstream>
#include <sstream>

shadow::ShaderManager::ShaderManager(const std::filesystem::path& shadersDirectory) : shadersDirectory(shadersDirectory) {}

bool shadow::ShaderManager::reworkShaderFiles()
{
    bool changed = false;
    // check for files that were modified
    for (const std::filesystem::directory_entry& file : std::filesystem::directory_iterator(shadersDirectory))
    {
        if (is_regular_file(file))
        {
            const std::filesystem::path& path = file.path();
            std::map<std::filesystem::path, ShaderFileInfo>::iterator it = shaderFileInfos.find(path);
            if (it != shaderFileInfos.end())
            {
                try
                {
                    std::filesystem::file_time_type timestamp = last_write_time(path);
                    if (it->second.timestamp != timestamp)
                    {
                        changed = true;
                        it->second.timestamp = timestamp;
                        it->second.content.clear();
                        it->second.references.clear();
                        it->second.modified = true;
                    }
                }
                catch (std::exception&) {}
            }
            else
            {
                std::string extension = path.filename().generic_string();
                size_t dotIndex = extension.find_last_of('.');
                if (dotIndex != std::string::npos)
                {
                    extension = extension.substr(dotIndex);
                    std::transform(extension.begin(), extension.end(), extension.begin(), [](auto c) { return std::tolower(c); });
                    for (const std::string& refExt : SHADER_EXTENSIONS)
                    {
                        if (refExt == extension)
                        {
                            changed = true;
                            SHADOW_DEBUG("Adding shader file '{}' to pool...", path.generic_string());
                            shaderFileInfos.emplace(path, ShaderFileInfo{ {}, {}, last_write_time(path), {}, true });
                            break;
                        }
                    }
                }
            }
        }
    }
    if (!changed)
    {
        for (std::map<std::string, ShaderTextInclude>::value_type& pair : shaderIncludes)
        {
            if (pair.second.modified)
            {
                changed = true;
                break;
            }
        }
        if (!changed)
        {
            return false;
        }
    }
    // gather references for modified files
    for (std::map<std::filesystem::path, ShaderFileInfo>::value_type& pair : shaderFileInfos)
    {
        if (pair.second.modified)
        {
            pair.second.references.clear();
            pair.second.includes.clear();
            std::ifstream stream(pair.first);
            std::string line;
            unsigned int lineCounter{};
            while (std::getline(stream, line))
            {
                ++lineCounter;
                ShadowUtils::trim(line);
                std::string includedFile{};
                if (line.rfind(INCLUDE_TEXT, 0) == 0)
                {
                    includedFile = line.substr(INCLUDE_LENGTH);
                }
                else if (line.rfind(INCLUDED_FROM_TEXT, 0) == 0)
                {
                    includedFile = line.substr(INCLUDED_FROM_LENGTH);
                }
                if (!includedFile.empty())
                {
                    bool fileFound = false;
                    for (std::map<std::filesystem::path, ShaderFileInfo>::value_type& innerPair : shaderFileInfos)
                    {
                        if (innerPair.first.filename() == includedFile)
                        {
                            fileFound = true;
                            if (isShaderFileRecursivelyReferenced(pair.first, innerPair.first))
                            {
                                SHADOW_WARN("Recursive reference of '{}' found in '{}' (line {})! This reference will NOT be included.",
                                    pair.first.generic_string(), innerPair.first.generic_string(), lineCounter);
                            }
                            else
                            {
                                SHADOW_TRACE("File '{}' includes file '{}' at line {}.", pair.first.generic_string(), innerPair.first.generic_string(), lineCounter);
                                pair.second.references.insert(innerPair.first);
                            }
                            break;
                        }
                    }
                    if (!fileFound)
                    {
                        bool isTextInclude = false;
                        for (std::map<std::string, ShaderTextInclude>::value_type& innerPair : shaderIncludes)
                        {
                            if (innerPair.first == includedFile)
                            {
                                isTextInclude = true;
                                break;
                            }
                        }
                        if (isTextInclude)
                        {
                            SHADOW_TRACE("File '{}' includes '{}' at line {}.", pair.first.generic_string(), includedFile, lineCounter);
                            pair.second.includes.insert(includedFile);
                        }
                        else
                        {
                            SHADOW_ERROR("File '{}' referenced in '{}' (line {}) was NOT found!", includedFile, pair.first.generic_string(), lineCounter);
                        }
                    }
                }
            }
        }
    }
    // rework modified status basing on references
    for (std::map<std::filesystem::path, ShaderFileInfo>::value_type& pair : shaderFileInfos)
    {
        if (!pair.second.modified && isShaderFileModified(pair.first))
        {
            pair.second.modified = true;
            pair.second.content.clear();
        }
    }
    // rebuild files if needed
    for (std::map<std::filesystem::path, ShaderFileInfo>::value_type& pair : shaderFileInfos)
    {
        if (pair.second.modified && !rebuildShaderFile(pair.first))
        {
            SHADOW_ERROR("Failed to rebuild shader file '{}'!", pair.first.generic_string());
        }
    }
    // clear modified status
    for (std::map<std::filesystem::path, ShaderFileInfo>::value_type& pair : shaderFileInfos)
    {
        pair.second.modified = false;
    }
    for (std::map<std::string, ShaderTextInclude>::value_type& pair : shaderIncludes)
    {
        pair.second.modified = false;
    }
    return true;
}

void shadow::ShaderManager::updateShaders() const
{
    for (const std::map<ShaderType, std::shared_ptr<GLShader>>::value_type& pair : shaders)
    {
        pair.second->update();
    }
}

void shadow::ShaderManager::updateVogelDisk(unsigned int shadowSamples, unsigned int penumbraSamples)
{
    assert(shadowSamples);
    assert(penumbraSamples);
    updateInclude(VOGEL_INCLUDE_TEXT, getVogelIncludeContent(shadowSamples, penumbraSamples));
}

std::string shadow::ShaderManager::getShaderFileContent(const std::filesystem::path& path)
{
    const std::map<std::filesystem::path, ShaderFileInfo>::iterator it = shaderFileInfos.find(path);
    if (it == shaderFileInfos.end())
    {
        return {};
    }
    return it->second.content;
}

std::shared_ptr<shadow::GLShader> shadow::ShaderManager::getShader(ShaderType shaderType)
{
    if (shaderType == ShaderType::None)
    {
        SHADOW_WARN("An empty shader was requested! This is probably a bug.");
        return {};
    }
    const std::map<ShaderType, std::shared_ptr<GLShader>>::iterator it = shaders.find(shaderType);
    if (it == shaders.end())
    {
        SHADOW_ERROR("Shader of type {} was not found!", shaderType);
        return {};
    }
    return it->second;
}

std::shared_ptr<shadow::UboMvp> shadow::ShaderManager::getUboMvp() const
{
    return uboMvp;
}

std::shared_ptr<shadow::UboMaterial> shadow::ShaderManager::getUboMaterial() const
{
    return uboMaterial;
}

std::shared_ptr<shadow::UboLights> shadow::ShaderManager::getUboLights() const
{
    return uboLights;
}

std::shared_ptr<shadow::UboWindow> shadow::ShaderManager::getUboWindow() const
{
    return uboWindow;
}

bool shadow::ShaderManager::rebuildShaderFile(const std::filesystem::path& path)
{
    const std::map<std::filesystem::path, ShaderFileInfo>::iterator it = shaderFileInfos.find(path);
    if (it == shaderFileInfos.end())
    {
        SHADOW_ERROR("File '{}' was not found in shaderFileInfos! This should NOT happen.", path.generic_string());
        return false;
    }
    if (!it->second.modified || !it->second.content.empty()) // if content is not empty, this means that the file was just rebuilt but is still marked as modified for the other files to know
    {
        return true;
    }
    SHADOW_DEBUG("Rebuilding shader file '{}'...", path.generic_string());
    std::ifstream stream(it->first);
    if (it->second.references.empty() && it->second.includes.empty())
    {
        it->second.content = std::string(std::istreambuf_iterator<char>(stream), std::istreambuf_iterator<char>());
    }
    else
    {
        for (const std::filesystem::path& refPath : it->second.references)
        {
            if (!rebuildShaderFile(refPath))
            {
                return false;
            }
        }
        std::vector<std::string> lines{};
        std::string line;
        while (std::getline(stream, line))
        {
            lines.push_back(line);
        }
        std::stringstream ss{};
        std::stringstream nonContentSs{};
        bool refillFile = false;
        for (size_t i = 0; i < lines.size();)
        {
            std::string trimmed = lines[i];
            ShadowUtils::trim(trimmed);
            if (trimmed.rfind(INCLUDE_TEXT, 0) == 0)
            {
                std::string includedFile = trimmed.substr(INCLUDE_LENGTH);
                bool includedFileFound = false;
                for (const std::map<std::filesystem::path, ShaderFileInfo>::value_type& pair : shaderFileInfos)
                {
                    if (pair.first.filename() == includedFile)
                    {
                        includedFileFound = true;
                        ss << INCLUDED_FROM_TEXT << includedFile << std::endl << pair.second.content << std::endl << END_INCLUDE_TEXT << includedFile << std::endl;
                        break;
                    }
                }
                if (!includedFileFound)
                {
                    bool isTextInclude = false;
                    for (const std::map<std::string, ShaderTextInclude>::value_type& pair : shaderIncludes)
                    {
                        if (pair.first == includedFile)
                        {
                            isTextInclude = true;
                            ss << INCLUDED_FROM_TEXT << includedFile << std::endl << pair.second.content << std::endl << END_INCLUDE_TEXT << includedFile << std::endl;
                            break;
                        }
                    }
                    if (!isTextInclude)
                    {
                        SHADOW_ERROR("Include file '{}' referenced in '{}':{} was NOT found!", includedFile, path.generic_string(), i + 1);
                        ss << INCLUDE_TEXT << includedFile << std::endl;
                    }
                }
                nonContentSs << trimmed << std::endl;
                ++i;
            }
            else if (trimmed.rfind(INCLUDED_FROM_TEXT, 0) == 0)
            {
                std::string includedFile = trimmed.substr(INCLUDED_FROM_LENGTH);
                size_t endIndex = std::string::npos;
                for (size_t j = i + 1; j < lines.size(); ++j)
                {
                    std::string tr = lines[j];
                    ShadowUtils::trim(tr);
                    if (tr.rfind(END_INCLUDE_TEXT, 0) == 0)
                    {
                        if (tr.substr(END_INCLUDE_LENGTH) == includedFile)
                        {
                            endIndex = j;
                            break;
                        }
                    }
                }
                if (endIndex == std::string::npos)
                {
                    SHADOW_ERROR("Unable to find inclusion end ('{}' included at line {} in '{}')!", includedFile, i + 1, path.generic_string());
                }
                else
                {
                    lines.erase(lines.begin() + i, lines.begin() + endIndex);
                    bool fileFound = false;
                    for (const std::map<std::filesystem::path, ShaderFileInfo>::value_type& pair : shaderFileInfos)
                    {
                        if (pair.first.filename() == includedFile)
                        {
                            fileFound = true;
                            ss << INCLUDED_FROM_TEXT << includedFile << std::endl << pair.second.content << std::endl << END_INCLUDE_TEXT << includedFile << std::endl;
                            break;
                        }
                    }
                    if (!fileFound)
                    {
                        bool isTextInclude = false;
                        for (const std::map<std::string, ShaderTextInclude>::value_type& pair : shaderIncludes)
                        {
                            if (pair.first == includedFile)
                            {
                                isTextInclude = true;
                                ss << INCLUDED_FROM_TEXT << includedFile << std::endl << pair.second.content << std::endl << END_INCLUDE_TEXT << includedFile << std::endl;
                                break;
                            }
                        }
                        if (!isTextInclude)
                        {
                            SHADOW_ERROR("Include file '{}' referenced in '{}':{} was NOT found!", includedFile, path.generic_string(), i + 1);
                            ss << INCLUDE_TEXT << includedFile << std::endl;
                        }
                    }
                }
                nonContentSs << INCLUDE_TEXT << includedFile << std::endl;
                ++i;
            }
            else if (trimmed.rfind(REFILL_TEXT, 0) == 0)
            {
                ss << lines[i] << std::endl;
                nonContentSs << lines[i] << std::endl;
                refillFile = true;
                SHADOW_DEBUG("Shader file '{}' is marked to be refilled.", path.generic_string());
                ++i;
            }
            else
            {
                ss << lines[i] << std::endl;
                nonContentSs << lines[i] << std::endl;
                ++i;
            }
        }
        it->second.content = ss.str();
        stream.close();
        SHADOW_DEBUG("Overwriting '{}' ({})...", path.generic_string(), refillFile);
        std::ofstream output(path, std::ios::trunc);
        if (refillFile)
        {
            output << it->second.content;
        }
        else
        {
            output << nonContentSs.str();
        }
        output.flush();
        it->second.timestamp = last_write_time(path);
    }
    return true;
}

bool shadow::ShaderManager::isShaderFileRecursivelyReferenced(const std::filesystem::path& path, const std::filesystem::path& searchPath)
{
    const std::map<std::filesystem::path, ShaderFileInfo>::iterator it = shaderFileInfos.find(searchPath);
    if (it == shaderFileInfos.end())
    {
        SHADOW_ERROR("File '{}' was not found in shaderFileInfos! This should NOT happen.", searchPath.generic_string());
        return false;
    }
    for (const std::filesystem::path& refPath : it->second.references)
    {
        if (refPath == path)
        {
            return true;
        }
        if (isShaderFileRecursivelyReferenced(path, refPath))
        {
            return true;
        }
    }
    return false;
}

bool shadow::ShaderManager::isShaderFileModified(const std::filesystem::path& path)
{
    const std::map<std::filesystem::path, ShaderFileInfo>::iterator it = shaderFileInfos.find(path);
    if (it == shaderFileInfos.end())
    {
        SHADOW_ERROR("File '{}' was not found in shaderFileInfos! This should NOT happen.", path.generic_string());
        return false;
    }
    if (it->second.modified)
    {
        return true;
    }
    for (const std::string& incl : it->second.includes)
    {
        if (shaderIncludes[incl].modified)
        {
            return true;
        }
    }
    for (const std::filesystem::path& refPath : it->second.references)
    {
        if (isShaderFileModified(refPath))
        {
            return true;
        }
    }
    return false;
}

void shadow::ShaderManager::loadShaders()
{
    SHADOW_DEBUG("Preparing shader includes...");
    prepareShaderIncludes();
    SHADOW_DEBUG("Reworking shader files...");
    reworkShaderFiles();
    SHADOW_DEBUG("Loading shaders...");
    shaders.emplace(ShaderType::Texture, std::shared_ptr<GLShader>(new GLShader(shadersDirectory, "Texture")));
    shaders.emplace(ShaderType::Material, std::shared_ptr<GLShader>(new GLShader(shadersDirectory, "Material")));
    shaders.emplace(ShaderType::DepthDir, std::shared_ptr<GLShader>(new GLShader(shadersDirectory, "DepthDir.vert", "Depth.frag")));
    shaders.emplace(ShaderType::DepthSpot, std::shared_ptr<GLShader>(new GLShader(shadersDirectory, "DepthSpot.vert", "Depth.frag")));
    shaders.emplace(ShaderType::DepthDirVSM, std::shared_ptr<GLShader>(new GLShader(shadersDirectory, "DepthDir.vert", "DepthVSM.frag")));
    shaders.emplace(ShaderType::DepthSpotVSM, std::shared_ptr<GLShader>(new GLShader(shadersDirectory, "DepthSpot.vert", "DepthVSM.frag")));
    shaders.emplace(ShaderType::DirPenumbra, std::shared_ptr<GLShader>(new GLShader(shadersDirectory, "DirPenumbra.vert", "DirPenumbra.frag")));
    shaders.emplace(ShaderType::SpotPenumbra, std::shared_ptr<GLShader>(new GLShader(shadersDirectory, "SpotPenumbra.vert", "SpotPenumbra.frag")));
    shaders.emplace(ShaderType::GaussianBlur, std::shared_ptr<GLShader>(new GLShader(shadersDirectory, "PostProcess.vert", "GaussianBlur.frag")));
    shaders.emplace(ShaderType::PostProcess, std::shared_ptr<GLShader>(new GLShader(shadersDirectory, "PostProcess")));
    shaders.emplace(ShaderType::InterleavedGradientNoise, std::shared_ptr<GLShader>(new GLShader(shadersDirectory, "PostProcess.vert", "InterleavedGradientNoise.frag")));
    shaders.emplace(ShaderType::ShadowOnly, std::shared_ptr<GLShader>(new GLShader(shadersDirectory, "ShadowOnly")));
    for (unsigned int i = 0U; i != static_cast<unsigned int>(ShaderType::ShaderTypeEnd); ++i)
    {
        const std::map<ShaderType, std::shared_ptr<GLShader>>::iterator it = shaders.find(static_cast<ShaderType>(i));
        if (it != shaders.end())
        {
            if (!it->second->createProgram())
            {
                SHADOW_ERROR("Failed to create shader of type {}!", it->first);
            }
        }
    }

    SHADOW_DEBUG("Creating UBOs...");
    uboMvp = std::make_shared<UboMvp>();
    uboMaterial = std::make_shared<UboMaterial>();
    DirectionalLightData dirLightData{};
    SpotLightData spotLightData{};
    uboLights = std::make_shared<UboLights>(
        std::make_shared<DirectionalLight>(dirLightData),
        std::make_shared<SpotLight>(spotLightData));
    uboWindow = std::make_shared<UboWindow>();
}

void shadow::ShaderManager::updateInclude(const std::string& inclName, const std::string& inclContent)
{
    for (std::map<std::string, ShaderTextInclude>::value_type& pair : shaderIncludes)
    {
        if (pair.first == inclName)
        {
            pair.second.content = inclContent;
            pair.second.modified = true;
            break;
        }
    }
}

void shadow::ShaderManager::addShaderInclude(const std::string& name, const std::string& content)
{
    shaderIncludes.emplace(name, ShaderTextInclude{ content, true });
}

std::string shadow::ShaderManager::getVogelIncludeContent(unsigned int shadowSamples, unsigned int penumbraSamples)
{
    std::stringstream ss{};
    ss << "#define VOGEL_SS " << shadowSamples << std::endl;
    ss << "#define VOGEL_PS " << penumbraSamples << std::endl << std::endl;
    std::vector<glm::vec2> shadowVogelDisk = getVogelDisk(shadowSamples);
    std::vector<glm::vec2> penumbraVogelDisk = getVogelDisk(penumbraSamples);
    ss << "vec2 shadowVogelDisk[VOGEL_SS] = vec2[](" << std::endl;
    for (unsigned int i = 0; i < shadowSamples; ++i)
    {
        ss << "    vec2(" << shadowVogelDisk[i].r << ", " << shadowVogelDisk[i].g << ")";
        if (i + 1U != shadowSamples)
        {
            ss << ",";
        }
        ss << std::endl;
    }
    ss << "    );" << std::endl << std::endl;
    ss << "vec2 penumbraVogelDisk[VOGEL_PS] = vec2[](" << std::endl;
    for (unsigned int i = 0; i < penumbraSamples; ++i)
    {
        ss << "    vec2(" << penumbraVogelDisk[i].r << ", " << penumbraVogelDisk[i].g << ")";
        if (i + 1U != penumbraSamples)
        {
            ss << ",";
        }
        ss << std::endl;
    }
    ss << "    );" << std::endl;
    return ss.str();
}

std::vector<glm::vec2> shadow::ShaderManager::getVogelDisk(unsigned int size) const
{
    float sizeSqrt = std::sqrt(static_cast<float>(size));
    std::vector<glm::vec2> result{};
    for (unsigned int i = 0; i < size; ++i)
    {
        result.emplace_back(
            std::sqrt(static_cast<float>(i) + 0.5f) / sizeSqrt,
            static_cast<float>(i) * 2.4f
        );
    }
    return result;
}

void shadow::ShaderManager::prepareShaderIncludes()
{
    addShaderInclude(VOGEL_INCLUDE_TEXT, getVogelIncludeContent(32U, 16U));
}
