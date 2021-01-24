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
    float padding;
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
#define PCF_KERNEL_SIZE 3
const int PCF_MAX = PCF_KERNEL_SIZE / 2, PCF_MIN = -PCF_MAX;
const float PCF_KERNEL_SQUARED = PCF_KERNEL_SIZE*PCF_KERNEL_SIZE;
const float TEX_SIZE_MULTIPLIER = 1.0f;

float calcShadow(float worldNdotL, vec4 lightSpacePos, sampler2D text)
{
    vec3 projCoords = (lightSpacePos.xyz / lightSpacePos.w) * 0.5 + 0.5;
    if(projCoords.z <= 1.0)
    {
        float closestDepth = texture(text, projCoords.xy).r;
        float currentDepth = projCoords.z;
        float bias = max(0.005 * (1.0 - worldNdotL), 0.00125);
        if(currentDepth - bias > closestDepth)
        {
            return 0.0;
        }
        vec2 texelSize = TEX_SIZE_MULTIPLIER/textureSize(text, 0);
        float shadow = 0.0;
        for(int x=PCF_MIN;x<=PCF_MAX;++x)
        {
            for(int y=PCF_MIN;y<=PCF_MAX;++y)
            {
                float pcfDepth = texture(text, projCoords.xy + vec2(x,y)*texelSize).r;
                shadow += currentDepth  - bias > pcfDepth ? 0.0 : 1.0;
            }
        }
        return (shadow/PCF_KERNEL_SQUARED);
    }
    return 1.0;
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
    float shadow = calcShadow(NdotL, fs_in.dirSpacePos, directionalShadow);
    vec3 H = normalize(V + L);
    float cosTheta = clamp(dot(H, V), 0.0, 1.0);
    vec3 F = fresnelSchlick(cosTheta, F0);
    float D = DistributionGGX(N, H, roughness);
    float G = GeometrySmith(N, V, L, roughness);
    vec3 specular = (F*D*G) / max(4.0 * NdotV * NdotL, 0.00001);
    vec3 kD = (vec3(1.0) - specular) * (1.0 - metallic);
    vec3 diffuse = kD * albedo / PI;
    return (diffuse + specular) * dirLightData.color * dirLightData.strength * NdotL * shadow;
}

vec3 getSpotLightColor(vec3 N, vec3 V, float NdotV, vec3 F0)
{
    if(spotLightData.strength == 0.0)
    {
        return vec3(0.0);
    }
    vec3 L = normalize(spotLightData.position - fs_in.pos);
    float NdotL = max(dot(N, L), 0.0);
    float shadow = calcShadow(NdotL, fs_in.spotSpacePos, spotShadow);
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
    return (diffuse + specular) * spotLightData.color * spotLightData.strength * attenuation * intensity * NdotL * shadow;
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
