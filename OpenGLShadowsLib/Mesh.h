#pragma once

#include "ShaderType.h"
#include "GLShader.h"

#include <memory>

namespace shadow
{
    class Mesh abstract
    {
    public:
        virtual ~Mesh() = default;
        virtual void draw(std::shared_ptr<GLShader> shader, glm::mat4 model) const = 0;
        virtual ShaderType getShaderType() const = 0;
    protected:
        Mesh() = default;
    };
}

