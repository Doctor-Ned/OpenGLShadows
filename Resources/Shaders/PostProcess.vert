#version 430 core
layout (location = 0) in vec2 pos;
layout (location = 1) in vec2 texCoords;

out VS_OUT
{
    vec2 texCoords;
} vs_out;

void main()
{
    vs_out.texCoords = texCoords;
    gl_Position = vec4(pos.x, pos.y, 0.0, 1.0);
}