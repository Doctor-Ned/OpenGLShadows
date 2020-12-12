#pragma once

#include "GLShader.h"

namespace shadow
{
    template<typename T>
    class UniformBufferObject abstract
    {
    public:
        virtual ~UniformBufferObject();
        void set(const T& value);
        bool bindTo(std::shared_ptr<GLShader> shader);
    protected:
        UniformBufferObject(gsl::cstring_span blockName, GLuint binding);
        void bufferSubData(void* data, GLsizeiptr size, GLintptr offset);
        GLuint binding{};
        GLuint uboId{};
        gsl::cstring_span blockName{};
    };

    template<typename T>
    inline UniformBufferObject<T>::~UniformBufferObject()
    {
        glDeleteBuffers(1, &uboId);
    }

    template<typename T>
    inline void UniformBufferObject<T>::set(const T& value)
    {
        bufferSubData(&value, sizeof(T), 0);
    }

    template<typename T>
    inline bool UniformBufferObject<T>::bindTo(std::shared_ptr<GLShader> shader)
    {
        GLuint index = glGetUniformBlockIndex(shader->getProgramId(), blockName.begin());
        if (index == GL_INVALID_INDEX)
        {
            SHADOW_ERROR("Uniform block '{}' not found in provided shader!", blockName.begin());
            return false;
        }
        glUniformBlockBinding(shader->getProgramId(), index, binding);
        return true;
    }

    template<typename T>
    inline UniformBufferObject<T>::UniformBufferObject(gsl::cstring_span blockName, GLuint binding) : blockName(blockName), binding(binding)
    {
        assert(!blockName.empty());
        glGenBuffers(1, &uboId);
        glBindBuffer(GL_UNIFORM_BUFFER, uboId);
        glBufferData(GL_UNIFORM_BUFFER, sizeof(T), nullptr, GL_STATIC_DRAW);
        glBindBuffer(GL_UNIFORM_BUFFER, 0);
        glBindBufferBase(GL_UNIFORM_BUFFER, binding, uboId);
    }
    template<typename T>
    inline void UniformBufferObject<T>::bufferSubData(void* data, GLsizeiptr size, GLintptr offset)
    {
        glBindBuffer(GL_UNIFORM_BUFFER, uboId);
        glBufferSubData(GL_UNIFORM_BUFFER, offset, size, data);
        glBindBuffer(GL_UNIFORM_BUFFER, 0);
    }
}
