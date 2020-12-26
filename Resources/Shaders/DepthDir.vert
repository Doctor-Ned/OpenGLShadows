#version 430 core
layout (location = 0) in vec3 pos;

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
    float padding;
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

void main()
{
    gl_Position = dirLightData.lightSpace * model * vec4(pos, 1.0);
}
