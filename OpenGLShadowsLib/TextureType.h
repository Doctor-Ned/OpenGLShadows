#pragma once

#include "ShadowLog.h"

#include "glad/glad.h"

enum class TextureType : GLuint
{
    // The bindings in shaders must match the values below!
    Albedo = 0U,
    Roughness = 1U,
    Metalness = 2U,
    Normal = 3U
};