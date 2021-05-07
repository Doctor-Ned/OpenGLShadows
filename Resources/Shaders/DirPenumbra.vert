#version 430 core
layout (location = 0) in vec3 pos;

//SHADOW>include UboMvp.glsl

//SHADOW>include LightStructs.glsl

//SHADOW>include UboLights.glsl

out VS_OUT
{
    vec4 dirSpacePos;
} vs_out;

void main()
{
    vec4 position = model * vec4(pos, 1.0);
    position.w = 1.0;
    vs_out.dirSpacePos = dirLightData.lightSpace * position;
    gl_Position = projection * view * position;
}
