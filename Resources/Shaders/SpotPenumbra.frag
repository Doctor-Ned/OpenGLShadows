#version 430 core

//SHADOW>include LightStructs.glsl

//SHADOW>include UboLights.glsl

//SHADOW>include ShadowVariants.glsl

layout(binding = 12) uniform sampler2D spotShadow;
#if SHADOW_MASTER
layout(binding = 14) uniform sampler2D ignTexture;
#endif

in VS_OUT
{
    vec4 spotSpacePos;
} fs_in;

out float outColor;

//SHADOW>include ShadowCalculations.glsl

void main()
{
    outColor = calcPenumbra(fs_in.spotSpacePos, spotLightData.nearZ, spotLightData.lightSize, spotShadow);
}
