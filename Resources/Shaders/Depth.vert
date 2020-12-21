#version 430 core
layout (location = 0) in vec3 pos;

layout (std140, binding = 0) uniform ModelViewProjection
{
    mat4 model;
    mat4 view;
    mat4 projection;
    vec3 viewPosition;
    float paddingMvp;
};

uniform mat4 lightSpaceMatrix;

void main()
{
    gl_Position = lightSpaceMatrix * model * vec4(pos, 1.0);
}