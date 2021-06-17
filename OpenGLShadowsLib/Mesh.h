#pragma once

#include "ShaderType.h"
#include "GLShader.h"

#include <memory>

//#define RENDER_SHADOW_ONLY

namespace shadow
{
    class Mesh abstract
    {
    public:
        virtual ~Mesh() = default;
        virtual void draw(std::shared_ptr<GLShader> shader) const = 0;
        virtual ShaderType getShaderType() const = 0;
    protected:
        Mesh() = default;
    };
}

