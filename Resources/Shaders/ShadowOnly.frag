#version 430 core

//SHADOW>include LightStructs.glsl

//SHADOW>include UboLights.glsl

//SHADOW>include ShadowVariants.glsl

#if SHADOW_MASTER || SHADOW_CHSS
layout(binding = 10) uniform sampler2D directionalShadow;
layout(binding = 11) uniform sampler2D directionalPenumbra;
layout(binding = 12) uniform sampler2D spotShadow;
layout(binding = 13) uniform sampler2D spotPenumbra;
#else
layout(binding = 10) uniform sampler2D directionalShadow;
layout(binding = 11) uniform sampler2D spotShadow;
#endif

in VS_OUT
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
} fs_in;

layout(binding = 3) uniform sampler2D normalTexture;

out vec4 outColor;

//SHADOW>include ShadowCalculations.glsl

vec3 getDirectionalLightColor(vec3 N)
{
    if(dirLightData.strength == 0.0)
    {
        return vec3(0.0);
    }
#if SHADOW_MASTER || SHADOW_CHSS
    float shadow = calcShadow(dot(fs_in.normal, -dirLightData.direction), fs_in.dirSpacePos, dirLightData.nearZ, dirLightData.lightSize, directionalShadow, directionalPenumbra);
#elif SHADOW_PCSS
    float shadow = calcShadow(dot(fs_in.normal, -dirLightData.direction), fs_in.dirSpacePos, dirLightData.nearZ, dirLightData.lightSize, directionalShadow);
#elif SHADOW_VSM
    float shadow = calcShadow(fs_in.dirSpacePos, directionalShadow);
#else
    float shadow = calcShadow(dot(fs_in.normal, -dirLightData.direction), fs_in.dirSpacePos, directionalShadow);
#endif
    vec3 L = normalize(-fs_in.tangentDirLightDirection);
    float NdotL = max(dot(N, L), 0.0);
    return dirLightData.color * dirLightData.strength * NdotL * (1.0 - shadow);
}

vec3 getSpotLightColor(vec3 N)
{
    if(spotLightData.strength == 0.0)
    {
        return vec3(0.0);
    }
#if SHADOW_MASTER || SHADOW_CHSS
    float shadow = calcShadow(dot(fs_in.normal, normalize(spotLightData.position - fs_in.pos)), fs_in.spotSpacePos, spotLightData.nearZ, spotLightData.lightSize, spotShadow, spotPenumbra);
#elif SHADOW_PCSS
    float shadow = calcShadow(dot(fs_in.normal, normalize(spotLightData.position - fs_in.pos)), fs_in.spotSpacePos, spotLightData.nearZ, spotLightData.lightSize, spotShadow);
#elif SHADOW_VSM
    float shadow = calcShadow(fs_in.spotSpacePos, spotShadow);
#else
    float shadow = calcShadow(dot(fs_in.normal, normalize(spotLightData.position - fs_in.pos)), fs_in.spotSpacePos, spotShadow);
#endif
    vec3 L = normalize(fs_in.tangentSpotLightPosition - fs_in.tangentFragPos);
    float NdotL = max(dot(N, L), 0.0);
    vec3 toLight = normalize(-fs_in.tangentSpotLightDirection);
    float theta = dot(L, toLight);
    float epsilon = spotLightData.innerCutOff - spotLightData.outerCutOff;
    float intensity = clamp((theta - spotLightData.outerCutOff) / epsilon, 0.0, 1.0);
    float dist = length(spotLightData.position - fs_in.pos);
    float attenuation = 1.0 / (dist * dist);
    return spotLightData.color * spotLightData.strength * attenuation * intensity * NdotL * (1.0-shadow);
}

void main()
{
    vec3 N = normalize(texture(normalTexture, fs_in.texCoords).rgb * 2.0 - 1.0);
    vec3 Lo = max(getDirectionalLightColor(N), vec3(0.0)) + max(getSpotLightColor(N), vec3(0.0));
    outColor = vec4(Lo, 1.0);
}
