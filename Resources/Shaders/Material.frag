#version 430 core

layout (std140, binding = 1) uniform Material {
    vec3 albedo;
    float roughness;
    float metallic;
};

in VS_OUT {
    vec3 pos;
    vec3 normal;
    vec3 viewPosition;
    vec2 texCoords;
} fs_in;

out vec4 outColor;

void main() {
    outColor = vec4(albedo, 1.0);
}