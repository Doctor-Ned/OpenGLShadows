#pragma once

namespace shadow
{
    enum class ShaderType : unsigned int
    {
        None,
        Material,
        Texture,
        DepthDir,
        DepthSpot,
        DepthDirVSM,
        DepthSpotVSM,
        PostProcess,
        ShaderTypeEnd
    };
}