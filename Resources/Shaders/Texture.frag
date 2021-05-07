#version 430 core

//SHADOW>includedfrom LightStructs.glsl
struct DirectionalLightData
{
    mat4 lightSpace;
    vec3 color;
    float strength;
    vec3 direction;
    float nearZ;
    vec2 padding;
    float farZ;
    float lightSize;
};

struct SpotLightData
{
    mat4 lightSpace;
    vec3 color;
    float strength;
    vec3 direction;
    float innerCutOff;
    vec3 position;
    float outerCutOff;
    float nearZ;
    float farZ;
    float lightSize;
    float padding;
};
//SHADOW>endinclude LightStructs.glsl

//SHADOW>includedfrom UboLights.glsl
layout (std140, binding = 2) uniform Lights
{
    DirectionalLightData dirLightData;
    SpotLightData spotLightData;
    vec3 paddingL;
    float ambient;
};
//SHADOW>endinclude UboLights.glsl

layout(binding = 10) uniform sampler2D directionalShadow;
layout(binding = 11) uniform sampler2D directionalPenumbra;
layout(binding = 12) uniform sampler2D spotShadow;
layout(binding = 13) uniform sampler2D spotPenumbra;

in VS_OUT
{
    vec3 pos;
    vec3 normal;
    vec3 viewPosition;
    vec2 texCoords;
    vec3 tangentFragPos;
    vec3 tangentDirLightDirection;
    vec3 tangentSpotLightDirection;
    vec3 tangentSpotLightPosition;
    vec3 toView;
    vec4 dirSpacePos;
    vec4 spotSpacePos;
} fs_in;

layout(binding = 0) uniform sampler2D albedoTexture;
layout(binding = 1) uniform sampler2D roughnessTexture;
layout(binding = 2) uniform sampler2D metalnessTexture;
layout(binding = 3) uniform sampler2D normalTexture;

out vec4 outColor;

//SHADOW>includedfrom PBRFunctions.glsl
const float PI = 3.14159265359;
  
float DistributionGGX(vec3 N, vec3 H, float roughness)
{
    float a      = roughness*roughness;
    float a2     = a*a;
    float NdotH  = max(dot(N, H), 0.0);
    float nom   = a2;
    float denom = (NdotH * NdotH * (a2 - 1.0) + 1.0);
    denom = PI * denom * denom;
    return nom / max(denom, 0.001);
}

float GeometrySchlickGGX(float NdotV, float roughness)
{
    float r = (roughness + 1.0);
    float k = (r*r) / 8.0;

    float nom   = NdotV;
    float denom = NdotV * (1.0 - k) + k;

    return nom / denom;
}

float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness)
{
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    float ggx2 = GeometrySchlickGGX(NdotV, roughness);
    float ggx1 = GeometrySchlickGGX(NdotL, roughness);

    return ggx1 * ggx2;
}

vec3 fresnelSchlick(float cosTheta, vec3 F0)
{
    return F0 + (1.0 - F0) * pow(max(1.0 - cosTheta, 0.0), 5.0);
}
//SHADOW>endinclude PBRFunctions.glsl

//SHADOW>includedfrom ShadowCalculations.glsl
// reference: https://maxest.gct-game.net/content/chss.pdf
// https://github.com/maxest/MaxestFramework/blob/5b06324aea21227fbbebd3257d41b75f76135578/samples/shadows/data/common.hlsl
// https://github.com/maxest/MaxestFramework/blob/5b06324aea21227fbbebd3257d41b75f76135578/samples/shadows/data/shadow_mask_ps.hlsl

//SHADOW>includedfrom UboWindow.glsl
layout (std140, binding = 3) uniform Window
{
    vec2 windowSize;
};
//SHADOW>endinclude UboWindow.glsl

#define VOGEL_SAMPLES_COUNT 32
#define VOGEL_PENUMBRA_SAMPLES_COUNT 16

const float VOGEL_SAMPLES_COUNT_SQRT = sqrt(VOGEL_SAMPLES_COUNT);
const float VOGEL_PENUMBRA_SAMPLES_COUNT_SQRT = sqrt(VOGEL_PENUMBRA_SAMPLES_COUNT);

vec2 sampleVogelDisk(int sampleIndex, float phi)
{
    float r = sqrt(sampleIndex + 0.5f) / VOGEL_SAMPLES_COUNT_SQRT;
    float theta = sampleIndex * 2.4f + phi;
    return vec2(r * cos(theta), r * sin(theta));
}
vec2 sampleVogelPenumbraDisk(int sampleIndex, float phi)
{
    float r = sqrt(sampleIndex + 0.5f) / VOGEL_PENUMBRA_SAMPLES_COUNT_SQRT;
    float theta = sampleIndex * 2.4f + phi;
    return vec2(r * cos(theta), r * sin(theta));
}

float interleavedGradientNoise(vec2 screenPos)
{
    const vec3 factors = vec3(0.06711056f, 0.00583715f, 52.9829189f);
    return fract(factors.z * sin(dot(screenPos.xy, factors.xy)));
}

float penumbraSize(float receiverDepth, float blockerDepth)
{
    return (receiverDepth-blockerDepth) / blockerDepth;
}

vec2 calcPenumbra(vec4 lightSpacePos, float nearZ, float lightSize, sampler2D text)
{
    vec3 projCoords = (lightSpacePos.xyz / lightSpacePos.w) * 0.5 + 0.5;
    if(projCoords.z > 1.0)
    {
        return vec2(0.0, 0.0);
    }
    float blockerDepth = 0.0;
    int numBlockers = 0;
    vec2 texCoords=projCoords.xy;
    float searchWidth = lightSize * (projCoords.z - nearZ) / projCoords.z;
    for(int i = 0; i < VOGEL_PENUMBRA_SAMPLES_COUNT; ++i)
    {
        float depth = texture(text,
        texCoords + sampleVogelPenumbraDisk(
            i, interleavedGradientNoise(gl_FragCoord.xy / windowSize)) * searchWidth).r;
        if(depth < projCoords.z)
        {
            blockerDepth += depth;
            ++numBlockers;
        }
    }
    if(numBlockers == 0)
    {
        return vec2(0.0, 0.0);
    }
    blockerDepth /= numBlockers;
    return vec2((projCoords.z - blockerDepth) / blockerDepth, 1.0);
}

float calcShadow(float worldNdotL, vec4 lightSpacePos, float nearZ, float lightSize, sampler2D text, sampler2D penumbraText)
{
    vec3 projCoords = (lightSpacePos.xyz / lightSpacePos.w) * 0.5 + 0.5;
    if(projCoords.z > 1.0)
    {
        return 0.0;
    }
    vec2 penumbraRatio = texture(penumbraText, gl_FragCoord.xy / windowSize).rg;
    if(penumbraRatio.g == 0.0)
    {
        return 0.0;
    }
    float filterRadiusUV = penumbraRatio.r * lightSize * nearZ / projCoords.z;
    float shadow = 0.0;
    for(int i = 0; i < VOGEL_SAMPLES_COUNT; ++i)
    {
        float depth = texture(text,
        projCoords.xy + sampleVogelDisk(
            i, interleavedGradientNoise(gl_FragCoord.xy / windowSize)) * filterRadiusUV).r;
        if(depth < projCoords.z - 0.008)
        {
            shadow += 1.0;
        }
    }
    shadow /= VOGEL_SAMPLES_COUNT;
    return shadow;
}

//SHADOW>endinclude ShadowCalculations.glsl

vec3 getDirectionalLightColor(vec3 N, vec3 V, float NdotV, vec3 F0, vec3 albedo, float roughness, float metallic)
{
    if(dirLightData.strength == 0.0)
    {
        return vec3(0.0);
    }
    float shadow = calcShadow(dot(fs_in.normal, -dirLightData.direction), fs_in.dirSpacePos, dirLightData.nearZ, dirLightData.lightSize, directionalShadow, directionalPenumbra);
    vec3 L = normalize(-fs_in.tangentDirLightDirection);
    float NdotL = max(dot(N, L), 0.0);
    vec3 H = normalize(V + L);
    float cosTheta = clamp(dot(H, V), 0.0, 1.0);
    vec3 F = fresnelSchlick(cosTheta, F0);
    float D = DistributionGGX(N, H, roughness);
    float G = GeometrySmith(N, V, L, roughness);
    vec3 specular = (F*D*G) / max(4.0 * NdotV * NdotL, 0.00001);
    vec3 kD = (vec3(1.0) - specular) * (1.0 - metallic);
    vec3 diffuse = kD * albedo / PI;
    return (diffuse + specular) * dirLightData.color * dirLightData.strength * NdotL * (1.0-shadow);
}

vec3 getSpotLightColor(vec3 N, vec3 V, float NdotV, vec3 F0, vec3 albedo, float roughness, float metallic)
{
    if(spotLightData.strength == 0.0)
    {
        return vec3(0.0);
    }
    float shadow = calcShadow(dot(fs_in.normal, normalize(spotLightData.position - fs_in.pos)), fs_in.spotSpacePos, spotLightData.nearZ, spotLightData.lightSize, spotShadow, spotPenumbra);
    vec3 L = normalize(fs_in.tangentSpotLightPosition - fs_in.tangentFragPos);
    float NdotL = max(dot(N, L), 0.0);
    vec3 toLight = normalize(-fs_in.tangentSpotLightDirection);
    float theta = dot(L, toLight);
    float epsilon = spotLightData.innerCutOff - spotLightData.outerCutOff;
    float intensity = clamp((theta - spotLightData.outerCutOff) / epsilon, 0.0, 1.0);
    vec3 H = normalize(V + L);
    float cosTheta = max(dot(H, V), 0.0);
    vec3 F = fresnelSchlick(cosTheta, F0);
    float D = DistributionGGX(N, H, roughness);
    float G = GeometrySmith(N, V, L, roughness);
    float dist = length(spotLightData.position - fs_in.pos);
    float attenuation = 1.0 / (dist * dist);
    vec3 specular = (F*D*G) / max(4.0 * NdotV * NdotL, 0.00001);
    vec3 kD = (vec3(1.0) - specular) * (1.0 - metallic);
    vec3 diffuse = kD * albedo / PI;
    return (diffuse + specular) * spotLightData.color * spotLightData.strength * attenuation * intensity * NdotL * (1.0-shadow);
}

void main()
{
    vec3 albedo = texture(albedoTexture, fs_in.texCoords).rgb;
    float roughness = texture(roughnessTexture, fs_in.texCoords).r;
    float metallic = texture(metalnessTexture, fs_in.texCoords).r;
    vec3 N = normalize(texture(normalTexture, fs_in.texCoords).rgb * 2.0 - 1.0);
    float NdotV = max(dot(N, fs_in.toView), 0.0);
    vec3 F0 = mix(vec3(0.04), albedo, metallic);
    vec3 Lo =
        albedo * ambient
        + max(getDirectionalLightColor(N, fs_in.toView, NdotV, F0, albedo, roughness, metallic), vec3(0.0))
        + max(getSpotLightColor(N, fs_in.toView, NdotV, F0, albedo, roughness, metallic), vec3(0.0));
    outColor = vec4(Lo, 1.0);
}
