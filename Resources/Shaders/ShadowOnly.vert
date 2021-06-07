#version 430 core
layout (location = 0) in vec3 pos;
layout (location = 1) in vec3 normal;
layout (location = 2) in vec2 texCoords;
layout (location = 3) in vec3 tangent;
layout (location = 4) in vec3 bitangent;

//SHADOW>include UboMvp.glsl

//SHADOW>include LightStructs.glsl

//SHADOW>include UboLights.glsl

out VS_OUT
{
    vec3 pos;
    vec3 normal;
    vec2 texCoords;
    vec3 tangentFragPos;
    vec3 tangentDirLightDirection;
    vec3 tangentSpotLightDirection;
    vec3 tangentSpotLightPosition;
    vec4 dirSpacePos;
    vec4 spotSpacePos;
} vs_out;

void main()
{
    vs_out.pos = vec3(model * vec4(pos, 1.0));
    vs_out.texCoords = texCoords;
    vec3 T = normalize(vec3(model * vec4(tangent, 0.0)));
    vec3 B = normalize(vec3(model * vec4(bitangent, 0.0)));
    vec3 N = normalize(vec3(model * vec4(normal, 0.0)));
    mat3 TBN = transpose(mat3(T,B,N));
    vs_out.tangentFragPos = TBN * vs_out.pos;
    vs_out.tangentDirLightDirection = TBN * dirLightData.direction;
    vs_out.tangentSpotLightDirection = TBN * spotLightData.direction;
    vs_out.tangentSpotLightPosition = TBN * spotLightData.position;
    vs_out.dirSpacePos = dirLightData.lightSpace * vec4(vs_out.pos, 1.0);
    vs_out.spotSpacePos = spotLightData.lightSpace * vec4(vs_out.pos, 1.0);
    mat3 normalMatrix = transpose(inverse(mat3(model)));
    vs_out.normal = normalize(normalMatrix * normal);
    gl_Position = projection * view * vec4(vs_out.pos, 1.0);
}
