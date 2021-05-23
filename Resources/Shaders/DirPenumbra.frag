#version 430 core

//SHADOW>include LightStructs.glsl

//SHADOW>include UboLights.glsl

layout(binding = 10) uniform sampler2D directionalShadow;

in VS_OUT
{
    vec4 dirSpacePos;
} fs_in;

out vec4 outColor;

//SHADOW>include ShadowCalculations.glsl

void main()
{
    outColor = vec4(calcPenumbra(fs_in.dirSpacePos, dirLightData.nearZ, dirLightData.lightSize, directionalShadow), 0.0, 0.0, 1.0);
}