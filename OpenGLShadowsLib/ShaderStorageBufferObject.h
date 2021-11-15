#pragma once

#include "GLShader.h"

namespace shadow
{
    template<typename T>
    class ShaderStorageBufferObject abstract
    {
    public:
        virtual ~ShaderStorageBufferObject();
        void set(gsl::span<uint8_t> data);
        virtual void set(T& data) = 0;
        bool isDeclaredIn(std::shared_ptr<GLShader> shader);
        gsl::cstring_span getBlockName() const;
    protected:
        ShaderStorageBufferObject(gsl::cstring_span blockName, GLuint binding, GLsizeiptr size);
        void bufferSubData(void* data, GLsizeiptr size, GLintptr offset);
        gsl::cstring_span blockName{};
        GLuint binding{};
        GLuint ssboId{};
    };

    template<typename T>
    inline ShaderStorageBufferObject<T>::~ShaderStorageBufferObject()
    {
        glDeleteBuffers(1, &ssboId);
    }

    template<typename T>
    inline void ShaderStorageBufferObject<T>::set(gsl::span<uint8_t> data)
    {
        bufferSubData(data.begin(), data.size_bytes(), 0);
    }

    template<typename T>
    inline bool ShaderStorageBufferObject<T>::isDeclaredIn(std::shared_ptr<GLShader> shader)
    {
        return glGetUniformBlockIndex(shader->getProgramId(), blockName.begin()) != GL_INVALID_INDEX;
    }

    template<typename T>
    inline gsl::cstring_span ShaderStorageBufferObject<T>::getBlockName() const
    {
        return blockName;
    }

    template<typename T>
    inline ShaderStorageBufferObject<T>::ShaderStorageBufferObject(gsl::cstring_span blockName, GLuint binding, GLsizeiptr size) : blockName(blockName), binding(binding)
    {
        assert(!blockName.empty());
        glGenBuffers(1, &ssboId);
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssboId);
        glBufferData(GL_SHADER_STORAGE_BUFFER, size, nullptr, GL_DYNAMIC_COPY);
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, binding, ssboId);
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
    }

    template<typename T>
    inline void ShaderStorageBufferObject<T>::bufferSubData(void* data, GLsizeiptr size, GLintptr offset)
    {
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssboId);
        glBufferSubData(GL_SHADER_STORAGE_BUFFER, offset, size, data);
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
    }
}
