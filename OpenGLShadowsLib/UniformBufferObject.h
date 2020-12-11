#pragma once

#include "ShadowLog.h"

#include "glad/glad.h"
#include <gsl/gsl-lite.hpp>

namespace shadow
{
    template<typename T>
    class UniformBufferObject abstract
    {
    public:
        virtual ~UniformBufferObject();
    protected:
        UniformBufferObject(gsl::cstring_span blockName, GLuint binding);
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
    inline UniformBufferObject<T>::UniformBufferObject(gsl::cstring_span blockName, GLuint binding) : blockName(blockName), binding(binding)
    {
        assert(!blockName.empty());
        glGenBuffers(1, &uboId);
        glBindBuffer(GL_UNIFORM_BUFFER, uboId);
        glBufferData(GL_UNIFORM_BUFFER, sizeof(T), nullptr, GL_STATIC_DRAW);
        glBindBuffer(GL_UNIFORM_BUFFER, 0);
        glBindBufferBase(GL_UNIFORM_BUFFER, binding, uboId);
    }
}
