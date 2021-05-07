#version 430 core

//SHADOW>include LightStructs.glsl

//SHADOW>include UboLights.glsl

layout(binding = 12) uniform sampler2D spotShadow;

in VS_OUT
{
    vec4 spotSpacePos;
} fs_in;

out vec4 outColor;

//SHADOW>include ShadowCalculations.glsl

void main()
{
    outColor = vec4(calcPenumbra(fs_in.spotSpacePos, spotLightData.nearZ, spotLightData.lightSize, spotShadow), 0.0, 0.0, 1.0);
}
