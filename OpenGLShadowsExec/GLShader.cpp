#include "GLShader.h"
#include "ShadowLog.h"

#include <cassert>
#include <fstream>

shadow::GLShader::GLShader(gsl::cstring_span shaderPath, gsl::cstring_span commonFileName)
    : vertexFile(to_string(shaderPath) + commonFileName.cbegin() + ".vs"), fragmentFile(to_string(shaderPath) + commonFileName.cbegin() + ".fs")
{}

shadow::GLShader::GLShader(gsl::cstring_span shaderPath, gsl::cstring_span vertexFile, gsl::cstring_span fragmentFile)
    : vertexFile(to_string(shaderPath) + vertexFile.cbegin()), fragmentFile(to_string(shaderPath) + fragmentFile.cbegin())
{}

shadow::GLShader::~GLShader()
{
    deleteProgram();
}

bool shadow::GLShader::createProgram()
{
    assert(!programId);
    SHADOW_DEBUG("Creating program using '{}' and '{}'...", vertexFile.generic_string(), fragmentFile.generic_string());
    if (!buildShader(vertexShader, GL_VERTEX_SHADER, vertexFile))
    {
        SHADOW_ERROR("Failed to build vertex shader '{}'!", vertexFile.generic_string());
        return false;
    }
    vertexTimestamp = last_write_time(vertexFile);
    if (!buildShader(fragmentShader, GL_FRAGMENT_SHADER, fragmentFile))
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
            vertexTimestamp = timestamp;
            SHADOW_INFO("Vertex shader '{}' was modified! Rebuilding...", vertexFile.generic_string());
            GLuint shader;
            if (buildShader(shader, GL_VERTEX_SHADER, vertexFile))
            {
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
                    SHADOW_ERROR("Failed to rebuild shader program, using the old build.");
                }
            } else
            {
                SHADOW_ERROR("Failed to rebuild vertex shader, using the old build.");
            }
        }
    }
    if (exists(fragmentFile))
    {
        std::filesystem::file_time_type timestamp = last_write_time(fragmentFile);
        if (timestamp != fragmentTimestamp)
        {
            fragmentTimestamp = timestamp;
            SHADOW_INFO("Fragment shader '{}' was modified! Rebuilding...", fragmentFile.generic_string());
            GLuint shader;
            if (buildShader(shader, GL_FRAGMENT_SHADER, fragmentFile))
            {
                GLuint program;
                SHADOW_DEBUG("Shader rebuilt! Rebuilding program...");
                if (buildProgram(program, vertexShader, shader))
                {
                    GLuint oldShader = fragmentShader, oldProgram = programId;
                    programId = program;
                    fragmentShader = shader;
                    glDeleteShader(oldShader);
                    glDeleteProgram(oldProgram);
                    SHADOW_DEBUG("Program rebuilt and replaced successfully!");
                } else
                {
                    SHADOW_ERROR("Failed to rebuild shader program, using the old build.");
                }
            } else
            {
                SHADOW_ERROR("Failed to rebuild fragment shader, using the old build.");
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

void shadow::GLShader::use() const
{
    assert(programId);
    glUseProgram(programId);
}

GLuint shadow::GLShader::getProgramId() const
{
    assert(programId);
    return programId;
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
    return true;
}

bool shadow::GLShader::buildShader(GLuint& shaderId, GLuint shaderType, const std::filesystem::path& path) const
{
    SHADOW_DEBUG("Building shader '{}' of type '{}'...", path.generic_string(), shaderType);
    if (!exists(path))
    {
        shaderId = 0U;
        SHADOW_ERROR("Shader file '{}' does not exist!", path.generic_string());
        return false;
    }
    shaderId = glCreateShader(shaderType);
    std::ifstream data(path, std::ios::binary | std::ios::ate);
    const std::streamsize fileSize = data.tellg();
    const GLint textSize = static_cast<GLint>(fileSize);
    assert(static_cast<std::streamsize>(textSize) == fileSize);
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
        return false;
    }
    return true;
}
