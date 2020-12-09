#pragma once

#include "glad/glad.h"
#include <gsl/gsl-lite.hpp>
#include <filesystem>

namespace shadow
{
    class GLShader final
    {
    public:
        GLShader(gsl::cstring_span shaderPath, gsl::cstring_span commonFileName);
        GLShader(gsl::cstring_span shaderPath, gsl::cstring_span vertexFile, gsl::cstring_span fragmentFile);
        ~GLShader();
        GLShader(GLShader&) = delete;
        GLShader(GLShader&&) = delete;
        GLShader& operator=(GLShader&) = delete;
        GLShader& operator=(GLShader&&) = delete;
        bool createProgram();
        void update();
        void deleteProgram();
        void use() const;
        GLuint getProgramId() const;
    private:
        bool buildProgram(GLuint& programId, GLuint vertexShader, GLuint fragmentShader) const;
        bool buildShader(GLuint& shaderId, GLuint shaderType, const std::filesystem::path& path) const;
        std::filesystem::path vertexFile{}, fragmentFile{};
        std::filesystem::file_time_type vertexTimestamp{}, fragmentTimestamp{};
        GLuint programId{ 0U }, vertexShader{ 0U }, fragmentShader{ 0U };
    };
}

