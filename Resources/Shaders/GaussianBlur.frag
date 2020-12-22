#version 430 core

in VS_OUT
{
    vec2 texCoords;
} fs_in;

layout(binding = 12) uniform sampler2D image;
uniform vec2 direction;
uniform vec2 resolution;

out vec4 outColor;

void main()
{
    // reference: https://github.com/Jam3/glsl-fast-gaussian-blur/blob/master/9.glsl
    outColor = vec4(0.0);
    vec2 off1 = vec2(1.3846153846) * direction;
    vec2 off2 = vec2(3.2307692308) * direction;
    outColor += texture(image, fs_in.texCoords) * 0.2270270270;
    outColor += texture(image, fs_in.texCoords + (off1 / resolution)) * 0.3162162162;
    outColor += texture(image, fs_in.texCoords - (off1 / resolution)) * 0.3162162162;
    outColor += texture(image, fs_in.texCoords + (off2 / resolution)) * 0.0702702703;
    outColor += texture(image, fs_in.texCoords - (off2 / resolution)) * 0.0702702703;
}