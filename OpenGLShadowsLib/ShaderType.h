#pragma once

#include "ShadowVariants.h"

namespace shadow
{
    enum class ShaderType : unsigned int
    {
        None,
        Material,
        Texture,
        DepthDir,
        DepthSpot,
#if SHADOW_VSM
        DepthDirVSM,
        DepthSpotVSM,
#endif
#if SHADOW_MASTER || SHADOW_CHSS
        DirPenumbra,
        SpotPenumbra,
#endif
        GaussianBlur,
        PostProcess,
        ShadowOnly,
        ShaderTypeEnd
    };
}