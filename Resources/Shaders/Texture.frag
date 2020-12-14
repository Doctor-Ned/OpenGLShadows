#version 430 core

struct DirectionalLightData
{
    vec3 color;
    float strength;
    vec3 direction;
};

struct SpotLightData
{
    vec3 color;
    float strength;
    vec3 direction;
    float innerCutOff;
    vec3 position;
    float outerCutOff;
};

layout (std430, binding = 2) buffer DirectionalLights
{
    DirectionalLightData directionalLights[];
};

layout (std430, binding = 3) buffer SpotLights
{
    SpotLightData spotLights[];
};

in VS_OUT
{
    vec3 pos;
    vec3 normal;
    vec3 viewPosition;
    vec2 texCoords;
} fs_in;

layout(binding = 0) uniform sampler2D albedoTexture;
layout(binding = 1) uniform sampler2D roughnessTexture;
layout(binding = 2) uniform sampler2D metalnessTexture;
layout(binding = 3) uniform sampler2D normalTexture;

out vec4 outColor;

void main()
{
    outColor = texture(albedoTexture, fs_in.texCoords);
}