#version 430 core
layout (location = 0) in vec3 pos;
layout (location = 1) in vec3 normal;

//SHADOW>includedfrom UboMvp.glsl
layout (std140, binding = 0) uniform ModelViewProjection
{
    mat4 model;
    mat4 view;
    mat4 projection;
    vec3 viewPosition;
    float paddingMvp;
};
//SHADOW>endinclude UboMvp.glsl

//SHADOW>includedfrom LightStructs.glsl
struct DirectionalLightData
{
    mat4 lightSpace;
    vec3 color;
    float strength;
    vec3 direction;
    float nearZ;
    vec2 padding;
    float farZ;
    float lightSize;
};

struct SpotLightData
{
    mat4 lightSpace;
    vec3 color;
    float strength;
    vec3 direction;
    float innerCutOff;
    vec3 position;
    float outerCutOff;
    float nearZ;
    float farZ;
    float lightSize;
    float padding;
};
//SHADOW>endinclude LightStructs.glsl

//SHADOW>includedfrom UboLights.glsl
layout (std140, binding = 2) uniform Lights
{
    DirectionalLightData dirLightData;
    SpotLightData spotLightData;
    vec3 paddingL;
    float ambient;
};
//SHADOW>endinclude UboLights.glsl

out VS_OUT
{
    vec4 dirSpacePos;
} vs_out;

void main()
{
    vec3 pos = vec3(model * vec4(pos, 1.0));
    vs_out.dirSpacePos = dirLightData.lightSpace * vec4(pos, 1.0);
    gl_Position = projection * view * vec4(pos, 1.0);
}
