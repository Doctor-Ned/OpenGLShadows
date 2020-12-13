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
        bool bindTo(std::shared_ptr<GLShader> shader);
        gsl::cstring_span getBlockName() const;
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
    inline bool UniformBufferObject<T>::bindTo(std::shared_ptr<GLShader> shader)
    {
        // basically the same as in ::isDeclaredIn() but we need it anyway so one line's copy is no harm here
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
