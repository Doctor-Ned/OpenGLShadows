#version 430 core
layout (location = 0) in vec3 pos;
layout (location = 1) in vec3 normal;
layout (location = 2) in vec2 texCoords;
layout (location = 3) in vec3 tangent;
layout (location = 4) in vec3 bitangent;

layout (std140, binding = 0) uniform ModelViewProjection
{
    mat4 model;
    mat4 view;
    mat4 projection;
    vec3 viewPosition;
    float paddingMvp;
};

struct DirectionalLightData
{
    vec3 color;
    float strength;
    vec3 direction;
    float padding;
};

struct SpotLightData
{
    vec3 color;
    float strength;
    vec3 direction;
    float innerCutOff;
    vec3 position;
    float outerCutOff;
};

layout (std140, binding = 2) uniform Lights
{
    DirectionalLightData dirLightData;
    SpotLightData spotLightData;
    vec3 paddingL;
    float ambient;
};

out VS_OUT
{
    vec3 pos;
    vec3 normal;
    vec3 viewPosition;
    vec2 texCoords;
    vec3 tangentViewPos;
    vec3 tangentFragPos;
    vec3 tangentSpotPos;
} vs_out;

void main()
{
    vs_out.pos = vec3(model * vec4(pos, 1.0));
    vs_out.viewPosition = vec3(model * vec4(viewPosition, 1.0));
    vs_out.texCoords = texCoords;
    mat3 normalMatrix = transpose(inverse(mat3(model)));
    vs_out.normal = normalize(normalMatrix * normal);
    vec3 T = normalize(normalMatrix * tangent);
    vec3 N = normalize(normalMatrix * normal);
    T = normalize(T - dot(T, N) * N);
    vec3 B = cross(N, T);
    mat3 TBN = mat3(T, B, N);
    vs_out.tangentSpotPos = TBN * spotLightData.position;
    vs_out.tangentViewPos = TBN * viewPosition;
    vs_out.tangentFragPos = TBN * vs_out.pos;
    gl_Position = projection * view * vec4(vs_out.pos, 1.0);
}