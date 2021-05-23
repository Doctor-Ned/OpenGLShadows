#version 430 core

//SHADOW>include LightStructs.glsl

//SHADOW>include UboLights.glsl

layout(binding = 10) uniform sampler2D directionalShadow;
layout(binding = 14) uniform sampler2D ignTexture;

in VS_OUT
{
    vec4 dirSpacePos;
} fs_in;

out float outColor;

//SHADOW>include ShadowCalculations.glsl

void main()
{
    outColor = calcPenumbra(fs_in.dirSpacePos, dirLightData.nearZ, dirLightData.lightSize, directionalShadow);
}
