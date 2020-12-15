#version 430 core

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

uniform sampler2D directionalShadow;
uniform sampler2D spotShadow;

in VS_OUT
{
    vec3 pos;
    vec3 normal;
    vec3 viewPosition;
    vec2 texCoords;
    vec3 tangentViewPos;
    vec3 tangentFragPos;
    vec3 tangentSpotPos;
} fs_in;

layout(binding = 0) uniform sampler2D albedoTexture;
layout(binding = 1) uniform sampler2D roughnessTexture;
layout(binding = 2) uniform sampler2D metalnessTexture;
layout(binding = 3) uniform sampler2D normalTexture;

out vec4 outColor;

const float PI = 3.14159265359;
  
float DistributionGGX(vec3 N, vec3 H, float roughness)
{
    float a      = roughness*roughness;
    float a2     = a*a;
    float NdotH  = max(dot(N, H), 0.0);
    float NdotH2 = NdotH*NdotH;
    float num   = a2;
    float denom = (NdotH2 * (a2 - 1.0) + 1.0);
    denom = PI * denom * denom;
    return num / denom;
}

float GeometrySchlickGGX(float NdotV, float roughness)
{
    float r = (roughness + 1.0);
    float k = (r*r) / 8.0;
    float num   = NdotV;
    float denom = NdotV * (1.0 - k) + k;
    return num / denom;
}

float GeometrySmith(float NdotV, float NdotL, float roughness)
{
    float ggx2  = GeometrySchlickGGX(NdotV, roughness);
    float ggx1  = GeometrySchlickGGX(NdotL, roughness);
    return ggx1 * ggx2;
}

vec3 fresnelSchlick(float cosTheta, vec3 F0)
{
    return F0 + (1.0 - F0) * pow(max(1.0 - cosTheta, 0.0), 5.0);
}

vec3 getDirectionalLightColor(vec3 N, vec3 V, float NdotV, vec3 F0, vec3 albedo, float roughness, float metallic)
{
    if(dirLightData.strength == 0.0)
    {
        return vec3(0.0);
    }
    vec3 L = normalize(-dirLightData.direction);
    vec3 H = normalize(V + L);
    float NdotL = max(dot(N, L), 0.0);
    float cosTheta = max(dot(V, H), 0.0);
    vec3 F = fresnelSchlick(cosTheta, F0);
    float D = DistributionGGX(N, H, roughness);
    float G = GeometrySchlickGGX(NdotV, NdotL);
    vec3 kD = mix(vec3(1.0) - F, vec3(0.0), metallic);
    vec3 diffuse = kD * albedo / PI;
    vec3 specular = (F*D*G) / max(4.0 * NdotV * NdotL, 0.00001);
    return (diffuse + specular) * dirLightData.color * NdotL * dirLightData.strength;
}

vec3 getSpotLightColor(vec3 N, vec3 V, float NdotV, vec3 F0, vec3 albedo, float roughness, float metallic)
{
    if(spotLightData.strength == 0.0)
    {
        return vec3(0.0);
    }
    vec3 toLight = normalize(-spotLightData.direction);
    vec3 L = normalize(spotLightData.position - fs_in.pos);
    float theta = dot(L, toLight);
    float epsilon = spotLightData.innerCutOff - spotLightData.outerCutOff;
    float intensity = clamp((theta - spotLightData.outerCutOff) / epsilon, 0.0, 1.0);
    vec3 H = normalize(V + L);
    float NdotL = max(dot(N, L), 0.0);
    float cosTheta = max(dot(V, H), 0.0);
    vec3 F = fresnelSchlick(cosTheta, F0);
    float D = DistributionGGX(N, H, roughness);
    float G = GeometrySchlickGGX(NdotV, NdotL);
    float dist = length(fs_in.tangentSpotPos - fs_in.tangentFragPos);
    float attenuation = 1.0 / (dist * dist);
    vec3 kD = mix(vec3(1.0) - F, vec3(0.0), metallic);
    vec3 diffuse = kD * albedo / PI;
    vec3 specular = (F*D*G) / max(4.0 * NdotV * NdotL, 0.00001);
    return (diffuse + specular) * spotLightData.color * spotLightData.strength * attenuation * intensity * NdotL;
}

void main()
{
    vec3 albedo = texture(albedoTexture, fs_in.texCoords).rgb;
    float roughness = texture(roughnessTexture, fs_in.texCoords).r;
    float metallic = texture(metalnessTexture, fs_in.texCoords).r;
    vec3 N = texture(normalTexture, fs_in.texCoords).rgb;
    N = normalize(N * 2.0 - 1.0);
    vec3 V = normalize(fs_in.tangentViewPos - fs_in.tangentFragPos);
    float NdotV = max(dot(N, V), 0.0);
    vec3 F0 = mix(vec3(0.04), albedo, metallic);
    vec3 Lo =
        albedo * ambient
        + max(getDirectionalLightColor(N, V, NdotV, F0, albedo, roughness, metallic), vec3(0.0))
        + max(getSpotLightColor(N, V, NdotV, F0, albedo, roughness, metallic), vec3(0.0));
    outColor = vec4(Lo, 1.0);
    //outColor = texture(albedoTexture, fs_in.texCoords);
}