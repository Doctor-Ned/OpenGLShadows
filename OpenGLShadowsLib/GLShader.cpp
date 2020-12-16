#include "GLShader.h"
#include "ShadowLog.h"

#include <glm/gtc/type_ptr.hpp>
#include <fstream>

shadow::GLShader::GLShader(std::filesystem::path shaderPath, gsl::cstring_span commonFileName)
    : vertexFile(shaderPath / (to_string(commonFileName) + ".vert")), fragmentFile(shaderPath / (to_string(commonFileName) + ".frag"))
{}

shadow::GLShader::GLShader(std::filesystem::path shaderPath, gsl::cstring_span vertexFile, gsl::cstring_span fragmentFile)
    : vertexFile(shaderPath / to_string(vertexFile)), fragmentFile(shaderPath / to_string(fragmentFile))
{}

shadow::GLShader::~GLShader()
{
    deleteProgram();
}

bool shadow::GLShader::createProgram()
{
    assert(!programId);
    SHADOW_DEBUG("Creating program using '{}' and '{}'...", vertexFile.generic_string(), fragmentFile.generic_string());
    if (buildShader(vertexShader, GL_VERTEX_SHADER, vertexFile) != ShaderBuildStatus::Success)
    {
        SHADOW_ERROR("Failed to build vertex shader '{}'!", vertexFile.generic_string());
        return false;
    }
    vertexTimestamp = last_write_time(vertexFile);
    if (buildShader(fragmentShader, GL_FRAGMENT_SHADER, fragmentFile) != ShaderBuildStatus::Success)
    {
        SHADOW_ERROR("Failed to build fragment shader '{}'!", fragmentFile.generic_string());
        deleteProgram();
        return false;
    }
    fragmentTimestamp = last_write_time(fragmentFile);
    if (!buildProgram(programId, vertexShader, fragmentShader))
    {
        SHADOW_ERROR("Failed to build shader program!");
        deleteProgram();
        return false;
    }
    return true;
}

void shadow::GLShader::update()
{
    assert(programId);
    if (exists(vertexFile))
    {
        std::filesystem::file_time_type timestamp = last_write_time(vertexFile);
        if (timestamp != vertexTimestamp)
        {
            SHADOW_INFO("Vertex shader '{}' was modified! Rebuilding...", vertexFile.generic_string());
            GLuint shader;
            switch (buildShader(shader, GL_VERTEX_SHADER, vertexFile))
            {
                case ShaderBuildStatus::Failed:
                    vertexTimestamp = timestamp;
                    SHADOW_ERROR("Failed to rebuild vertex shader, using the old build.");
                    break;
                case ShaderBuildStatus::Success:
                {
                    vertexTimestamp = timestamp;
                    GLuint program;
                    SHADOW_DEBUG("Shader rebuilt! Rebuilding program...");
                    if (buildProgram(program, shader, fragmentShader))
                    {
                        GLuint oldShader = vertexShader, oldProgram = programId;
                        programId = program;
                        vertexShader = shader;
                        glDeleteShader(oldShader);
                        glDeleteProgram(oldProgram);
                        SHADOW_DEBUG("Program rebuilt and replaced successfully!");
                    } else
                    {
                        glDeleteShader(shader);
                        SHADOW_ERROR("Failed to rebuild shader program, using the old build.");
                    }
                    break;
                }
                default:
                    break;
            }
        }
    }
    if (exists(fragmentFile))
    {
        std::filesystem::file_time_type timestamp = last_write_time(fragmentFile);
        if (timestamp != fragmentTimestamp)
        {
            SHADOW_INFO("Fragment shader '{}' was modified! Rebuilding...", fragmentFile.generic_string());
            GLuint shader;
            switch (buildShader(shader, GL_FRAGMENT_SHADER, fragmentFile))
            {
                case ShaderBuildStatus::Failed:
                    fragmentTimestamp = timestamp;
                    SHADOW_ERROR("Failed to rebuild fragment shader, using the old build.");
                    break;
                case ShaderBuildStatus::Success:
                {
                    fragmentTimestamp = timestamp;
                    GLuint program;
                    SHADOW_DEBUG("Shader rebuilt! Rebuilding program...");
                    if (buildProgram(program, vertexShader, shader))
                    {
                        GLuint oldShader = fragmentShader, oldProgram = programId;
                        programId = program;
                        fragmentShader = shader;
                        glDeleteShader(oldShader);
                        glDeleteProgram(oldProgram);
                        SHADOW_DEBUG("Program rebuilt as {} and replaced successfully!", programId);
                    } else
                    {
                        glDeleteShader(shader);
                        SHADOW_ERROR("Failed to rebuild shader program, using the old build.");
                    }
                    break;
                }
                default:
                    break;
            }
        }
    }
}

void shadow::GLShader::deleteProgram()
{
    if (vertexShader)
    {
        glDeleteShader(vertexShader);
        vertexShader = 0U;
    }
    if (fragmentShader)
    {
        glDeleteShader(fragmentShader);
        fragmentShader = 0U;
    }
    if (programId)
    {
        glDeleteProgram(programId);
        programId = 0U;
    }
}

GLint shadow::GLShader::getUniformLocation(gsl::cstring_span name) const
{
    assert(programId);
    GLint result = glGetUniformLocation(programId, name.cbegin());
    if (result == -1)
    {
        SHADOW_ERROR("Uniform '{}' was not found in program {} ('{}', '{}')!",
                     name.cbegin(), programId, vertexFile.generic_string(), fragmentFile.generic_string());
    }
    return result;
}

void shadow::GLShader::setBool(gsl::cstring_span name, bool value) const
{
    if (GLint location = getUniformLocation(name); location != -1)
    {
        glUniform1i(location, static_cast<int>(value));
    }
}

void shadow::GLShader::setInt(gsl::cstring_span name, int value) const
{
    if (GLint location = getUniformLocation(name); location != -1)
    {
        glUniform1i(location, value);
    }
}

void shadow::GLShader::setFloat(gsl::cstring_span name, float value) const
{
    if (GLint location = getUniformLocation(name); location != -1)
    {
        glUniform1f(location, value);
    }
}

void shadow::GLShader::setVec2(gsl::cstring_span name, glm::vec2 value) const
{
    if (GLint location = getUniformLocation(name); location != -1)
    {
        glUniform2f(location, value.x, value.y);
    }
}

void shadow::GLShader::setVec3(gsl::cstring_span name, glm::vec3 value) const
{
    if (GLint location = getUniformLocation(name); location != -1)
    {
        glUniform3f(location, value.x, value.y, value.z);
    }
}

void shadow::GLShader::setVec4(gsl::cstring_span name, glm::vec4 value) const
{
    if (GLint location = getUniformLocation(name); location != -1)
    {
        glUniform4f(location, value.x, value.y, value.z, value.w);
    }
}

void shadow::GLShader::setMat2(gsl::cstring_span name, glm::mat2 value) const
{
    if (GLint location = getUniformLocation(name); location != -1)
    {
        glUniformMatrix2fv(location, 1, GL_FALSE, value_ptr(value));
    }
}

void shadow::GLShader::setMat3(gsl::cstring_span name, glm::mat3 value) const
{
    if (GLint location = getUniformLocation(name); location != -1)
    {
        glUniformMatrix3fv(location, 1, GL_FALSE, value_ptr(value));
    }
}

void shadow::GLShader::setMat4(gsl::cstring_span name, glm::mat4 value) const
{
    if (GLint location = getUniformLocation(name); location != -1)
    {
        glUniformMatrix4fv(location, 1, GL_FALSE, value_ptr(value));
    }
}

void shadow::GLShader::setModel(glm::mat4 model) const
{
    setMat4("model", model);
}

void shadow::GLShader::setLightSpaceMatrix(glm::mat4 lightSpaceMatrix) const
{
    setMat4("lightSpaceMatrix", lightSpaceMatrix);
}

bool shadow::GLShader::buildProgram(GLuint& programId, GLuint vertexShader, GLuint fragmentShader) const
{
    assert(vertexShader);
    assert(fragmentShader);
    SHADOW_DEBUG("Building program using '{}' and '{}'...", vertexFile.generic_string(), fragmentFile.generic_string());
    programId = glCreateProgram();
    glAttachShader(programId, vertexShader);
    glAttachShader(programId, fragmentShader);
    glLinkProgram(programId);
    GLint isFine;
    glGetProgramiv(programId, GL_LINK_STATUS, &isFine);
    if (!isFine)
    {
        GLint maxLength;
        glGetProgramiv(programId, GL_INFO_LOG_LENGTH, &maxLength);
        GLchar* errorMsg = new GLchar[maxLength];
        glGetProgramInfoLog(programId, maxLength, &maxLength, errorMsg);
        SHADOW_ERROR("Failed to link program! {}", errorMsg);
        delete[] errorMsg;
        glDeleteProgram(programId);
        programId = 0U;
        return false;
    }
    SHADOW_DEBUG("Program built as {}!", programId);
    return true;
}

shadow::ShaderBuildStatus shadow::GLShader::buildShader(GLuint& shaderId, GLuint shaderType, const std::filesystem::path& path) const
{
    SHADOW_DEBUG("Building shader '{}' of type '{}'...", path.generic_string(), shaderType);
    if (!exists(path))
    {
        shaderId = 0U;
        SHADOW_ERROR("Shader file '{}' does not exist!", path.generic_string());
        return ShaderBuildStatus::Failed;
    }
    shaderId = glCreateShader(shaderType);
    std::ifstream data(path, std::ios::binary | std::ios::ate);
    const std::streamsize fileSize = data.tellg();
    if (fileSize <= 0)
    {
        SHADOW_WARN("Shader file '{}' is probably still being written, postponing rebuild...", path.generic_string());
        return ShaderBuildStatus::Unavailable;
    }
    const GLint textSize = static_cast<GLint>(fileSize);
    assert(static_cast<std::streamsize>(textSize) == fileSize);
    assert(textSize > 0);
    data.seekg(0, std::ios::beg);
    GLchar* text = new GLchar[textSize];
    data.read(text, fileSize);
    glShaderSource(shaderId, 1, &text, &textSize);
    delete[] text;
    glCompileShader(shaderId);
    GLint isFine;
    glGetShaderiv(shaderId, GL_COMPILE_STATUS, &isFine);
    if (!isFine)
    {
        GLint maxLength;
        glGetShaderiv(shaderId, GL_INFO_LOG_LENGTH, &maxLength);
        GLchar* errorMsg = new GLchar[maxLength];
        glGetShaderInfoLog(shaderId, maxLength, &maxLength, errorMsg);
        SHADOW_ERROR("Failed to compile shader! {}", errorMsg);
        delete[] errorMsg;
        glDeleteShader(shaderId);
        shaderId = 0U;
        return ShaderBuildStatus::Failed;
    }
    return ShaderBuildStatus::Success;
}
