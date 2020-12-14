#version 430 core

layout (std140, binding = 1) uniform Material {
    vec3 albedo;
    float roughness;
    float metallic;
};

struct DirectionalLightData {
    vec3 color;
    float strength;
    vec3 direction;
};

struct SpotLightData {
    vec3 color;
    float strength;
    vec3 direction;
    float innerCutOff;
    vec3 position;
    float outerCutOff;
};

layout (std430, binding = 2) buffer DirectionalLights {
    DirectionalLightData directionalLights[];
};

layout (std430, binding = 3) buffer SpotLights {
    SpotLightData spotLights[];
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