#version 430 core

in VS_OUT
{
    vec2 texCoords;
} fs_in;

layout (binding = 0) uniform sampler2D screenTexture;

out vec4 outColor;

void main()
{
    outColor = vec4(texture(screenTexture, fs_in.texCoords).rgb, 1.0);
}