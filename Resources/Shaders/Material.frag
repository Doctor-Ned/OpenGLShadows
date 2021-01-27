#version 430 core

//SHADOW>includedfrom UboMaterial.glsl
layout (std140, binding = 1) uniform Material
{
    vec3 albedo;
    float roughness;
    vec3 paddingM;
    float metallic;
};
//SHADOW>endinclude UboMaterial.glsl

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
layout(binding = 11) uniform sampler2D spotShadow;

in VS_OUT
{
    vec3 pos;
    vec3 normal;
    vec3 viewPosition;
    vec3 toView;
    vec4 dirSpacePos;
    vec4 spotSpacePos;
} fs_in;

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
// reference: http://developer.download.nvidia.com/whitepapers/2008/PCSS_Integration.pdf

#define PCSS_MAX_BLOCKERS 32 // 16 or 32

#define PCSS_BLOCKERS 32 // within range (0;PCSS_MAX_BLOCKERS]
#define PCSS_FILTER_SIZE 32 // within range (0;PCSS_MAX_BLOCKERS]

#if PCSS_BLOCKERS <= 0 || PCSS_BLOCKERS > PCSS_MAX_BLOCKERS
#error Invalid PCSS_BLOCKERS value!
#endif

#if PCSS_FILTER_SIZE <= 0 || PCSS_FILTER_SIZE > PCSS_MAX_BLOCKERS
#error Invalid PCSS_FILTER_SIZE value!
#endif

#if PCSS_MAX_BLOCKERS == 32
vec2 POISSON_DISK[PCSS_MAX_BLOCKERS] = {
    vec2(0.06407013, 0.05409927),
    vec2(0.7366577, 0.5789394),
    vec2(-0.6270542, -0.5320278),
    vec2(-0.4096107, 0.8411095),
    vec2(0.6849564, -0.4990818),
    vec2(-0.874181, -0.04579735),
    vec2(0.9989998, 0.0009880066),
    vec2(-0.004920578, -0.9151649),
    vec2(0.1805763, 0.9747483),
    vec2(-0.2138451, 0.2635818),
    vec2(0.109845, 0.3884785),
    vec2(0.06876755, -0.3581074),
    vec2(0.374073, -0.7661266),
    vec2(0.3079132, -0.1216763),
    vec2(-0.3794335, -0.8271583),
    vec2(-0.203878, -0.07715034),
    vec2(0.5912697, 0.1469799),
    vec2(-0.88069, 0.3031784),
    vec2(0.5040108, 0.8283722),
    vec2(-0.5844124, 0.5494877),
    vec2(0.6017799, -0.1726654),
    vec2(-0.5554981, 0.1559997),
    vec2(-0.3016369, -0.3900928),
    vec2(-0.5550632, -0.1723762),
    vec2(0.925029, 0.2995041),
    vec2(-0.2473137, 0.5538505),
    vec2(0.9183037, -0.2862392),
    vec2(0.2469421, 0.6718712),
    vec2(0.3916397, -0.4328209),
    vec2(-0.03576927, -0.6220032),
    vec2(-0.04661255, 0.7995201),
    vec2(0.4402924, 0.3640312),
};
#elif PCSS_MAX_BLOCKERS == 16
vec2 POISSON_DISK[PCSS_MAX_BLOCKERS] =
{
    vec2(-0.94201624, -0.39906216),
    vec2(0.94558609, -0.76890725),
    vec2(-0.094184101, -0.92938870),
    vec2(0.34495938, 0.29387760),
    vec2(-0.91588581, 0.45771432),
    vec2(-0.81544232, -0.87912464),
    vec2(-0.38277543, 0.27676845),
    vec2(0.97484398, 0.75648379),
    vec2(0.44323325, -0.97511554),
    vec2(0.53742981, -0.47373420),
    vec2(-0.26496911, -0.41893023),
    vec2(0.79197514, 0.19090188),
    vec2(-0.24188840, 0.99706507),
    vec2(-0.81409955, 0.91437590),
    vec2(0.19984126, 0.78641367),
    vec2(0.14383161, -0.14100790)
};
#else
#error Invalid PCSS_MAX_BLOCKERS value!
#endif

float penumbraSize(float receiverDepth, float blockerDepth)
{
    return (receiverDepth-blockerDepth) / blockerDepth;
}

float calcShadow(float worldNdotL, vec4 lightSpacePos, float nearZ, float lightSize, sampler2D text)
{
    vec3 projCoords = (lightSpacePos.xyz / lightSpacePos.w) * 0.5 + 0.5;
    if(projCoords.z > 1.0)
    {
        return 0.0;
    }
    float blockerDepth = 0.0;
    int numBlockers = 0;
    vec2 texCoords=projCoords.xy;
    float searchWidth = lightSize * (projCoords.z - nearZ) / projCoords.z;
    for(int i=0;i<PCSS_BLOCKERS;++i)
    {
        float depth = texture(text, texCoords + POISSON_DISK[i] * searchWidth).r;
        if(depth < projCoords.z)
        {
            blockerDepth += depth;
            ++numBlockers;
        }
    }
    if(numBlockers == 0)
    {
        return 0.0;
    }
    blockerDepth /= numBlockers;
    float penumbraRatio = (projCoords.z - blockerDepth) / blockerDepth;
    float filterRadiusUV = penumbraRatio * lightSize * nearZ / projCoords.z;
    float shadow = 0.0;
    for(int i=0;i<PCSS_FILTER_SIZE;++i)
    {
        float depth = texture(text, projCoords.xy + POISSON_DISK[i] * filterRadiusUV).r;
        if(depth < projCoords.z - 0.008)
        {
            shadow += 1.0;
        }
    }
    shadow /= PCSS_FILTER_SIZE;
    return shadow;
}
//SHADOW>endinclude ShadowCalculations.glsl

vec3 getDirectionalLightColor(vec3 N, vec3 V, float NdotV, vec3 F0)
{
    if(dirLightData.strength == 0.0)
    {
        return vec3(0.0);
    }
    vec3 L = normalize(-dirLightData.direction);
    float NdotL = max(dot(N, L), 0.0);
    float shadow = calcShadow(NdotL, fs_in.dirSpacePos, dirLightData.nearZ, dirLightData.lightSize, directionalShadow);
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

vec3 getSpotLightColor(vec3 N, vec3 V, float NdotV, vec3 F0)
{
    if(spotLightData.strength == 0.0)
    {
        return vec3(0.0);
    }
    vec3 L = normalize(spotLightData.position - fs_in.pos);
    float NdotL = max(dot(N, L), 0.0);
    float shadow = calcShadow(NdotL, fs_in.spotSpacePos, spotLightData.nearZ, spotLightData.lightSize, spotShadow);
    vec3 toLight = normalize(-spotLightData.direction);
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
    float NdotV = max(dot(fs_in.normal, fs_in.toView), 0.0);
    vec3 F0 = mix(vec3(0.04), albedo, metallic);
    vec3 Lo =
        albedo * ambient
        + getDirectionalLightColor(fs_in.normal, fs_in.toView, NdotV, F0)
        + getSpotLightColor(fs_in.normal, fs_in.toView, NdotV, F0);
    outColor = vec4(Lo, 1.0);
}
