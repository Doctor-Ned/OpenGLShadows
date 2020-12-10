#pragma once

#include "glad/glad.h"
#include <glm/glm.hpp>
#include <gsl/gsl-lite.hpp>
#include <filesystem>
#include <cassert>

namespace shadow
{
    class GLShader final
    {
    public:
        ~GLShader();
        GLShader(GLShader&) = delete;
        GLShader(GLShader&&) = delete;
        GLShader& operator=(GLShader&) = delete;
        GLShader& operator=(GLShader&&) = delete;
        bool createProgram();
        void update();
        void deleteProgram();
        inline void use() const;
        inline GLuint getProgramId() const;
        GLint getUniformLocation(gsl::cstring_span name) const;
        void setBool(gsl::cstring_span name, bool value) const;
        void setInt(gsl::cstring_span name, int value) const;
        void setFloat(gsl::cstring_span name, float value) const;
        void setVec2(gsl::cstring_span name, glm::vec2 value) const;
        void setVec3(gsl::cstring_span name, glm::vec3 value) const;
        void setVec4(gsl::cstring_span name, glm::vec4 value) const;
        void setMat2(gsl::cstring_span name, glm::mat2 value) const;
        void setMat3(gsl::cstring_span name, glm::mat3 value) const;
        void setMat4(gsl::cstring_span name, glm::mat4 value) const;
    private:
        friend class ResourceManager;
        GLShader(gsl::cstring_span shaderPath, gsl::cstring_span commonFileName);
        GLShader(gsl::cstring_span shaderPath, gsl::cstring_span vertexFile, gsl::cstring_span fragmentFile);
        bool buildProgram(GLuint& programId, GLuint vertexShader, GLuint fragmentShader) const;
        bool buildShader(GLuint& shaderId, GLuint shaderType, const std::filesystem::path& path) const;
        std::filesystem::path vertexFile{}, fragmentFile{};
        std::filesystem::file_time_type vertexTimestamp{}, fragmentTimestamp{};
        GLuint programId{ 0U }, vertexShader{ 0U }, fragmentShader{ 0U };
    };

    inline void GLShader::use() const
    {
        assert(programId);
        glUseProgram(programId);
    }

    inline GLuint GLShader::getProgramId() const
    {
        assert(programId);
        return programId;
    }
}

