#version 430 core

layout (std140, binding = 1) uniform Material
{
    vec3 albedo;
    float roughness;
    float metallic;
};

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

layout (std140, binding = 2) uniform Lights
{
    DirectionalLightData dirLightData;
    SpotLightData spotLightData;
};

uniform sampler2D directionalShadow;
uniform sampler2D spotShadow;

in VS_OUT
{
    vec3 pos;
    vec3 normal;
    vec3 viewPosition;
} fs_in;

out vec4 outColor;

void main()
{
    outColor = vec4(albedo, 1.0);
}