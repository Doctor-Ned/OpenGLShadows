#version 430 core
layout (location = 0) in vec3 pos;

//SHADOW>include UboMvp.glsl

//SHADOW>include LightStructs.glsl

//SHADOW>include UboLights.glsl

void main()
{
    gl_Position = dirLightData.lightSpace * model * vec4(pos, 1.0);
}
