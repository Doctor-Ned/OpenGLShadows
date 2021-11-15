#pragma once

#include "GLShader.h"

namespace shadow
{
    template<typename T>
    class UniformBufferObject abstract
    {
    public:
        virtual ~UniformBufferObject();
        void set(T& value);
        bool isDeclaredIn(std::shared_ptr<GLShader> shader);
        gsl::cstring_span getBlockName() const;
    protected:
        UniformBufferObject(gsl::cstring_span blockName, GLuint binding);
        void bufferSubData(void* data, GLsizeiptr size, GLintptr offset);
        gsl::cstring_span blockName{};
        GLuint binding{};
        GLuint uboId{};
    };

    template<typename T>
    inline UniformBufferObject<T>::~UniformBufferObject()
    {
        glDeleteBuffers(1, &uboId);
    }

    template<typename T>
    inline void UniformBufferObject<T>::set(T& value)
    {
        bufferSubData(&value, sizeof(T), 0);
    }

    template<typename T>
    inline bool UniformBufferObject<T>::isDeclaredIn(std::shared_ptr<GLShader> shader)
    {
        return glGetUniformBlockIndex(shader->getProgramId(), blockName.begin()) != GL_INVALID_INDEX;
    }

    template<typename T>
    inline gsl::cstring_span UniformBufferObject<T>::getBlockName() const
    {
        return blockName;
    }

    template<typename T>
    inline UniformBufferObject<T>::UniformBufferObject(gsl::cstring_span blockName, GLuint binding) : blockName(blockName), binding(binding)
    {
        assert(!blockName.empty());
        glGenBuffers(1, &uboId);
        glBindBuffer(GL_UNIFORM_BUFFER, uboId);
        glBufferData(GL_UNIFORM_BUFFER, sizeof(T), nullptr, GL_STATIC_DRAW);
        glBindBufferBase(GL_UNIFORM_BUFFER, binding, uboId);
        glBindBuffer(GL_UNIFORM_BUFFER, 0);
    }

    template<typename T>
    inline void UniformBufferObject<T>::bufferSubData(void* data, GLsizeiptr size, GLintptr offset)
    {
        glBindBuffer(GL_UNIFORM_BUFFER, uboId);
        glBufferSubData(GL_UNIFORM_BUFFER, offset, size, data);
        glBindBuffer(GL_UNIFORM_BUFFER, 0);
    }
}
