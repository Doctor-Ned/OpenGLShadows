#version 430 core

in VS_OUT {
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

void main() {
    outColor = texture(albedoTexture, fs_in.texCoords);
}