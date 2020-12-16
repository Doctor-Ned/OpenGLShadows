#version 430 core

layout (std140, binding = 1) uniform Material
{
    vec3 albedo;
    float roughness;
    vec3 paddingM;
    float metallic;
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

uniform sampler2D directionalShadow;
uniform sampler2D spotShadow;

in VS_OUT
{
    vec3 pos;
    vec3 normal;
    vec3 viewPosition;
    vec3 toView;
} fs_in;

out vec4 outColor;

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

vec3 getDirectionalLightColor(vec3 N, vec3 V, float NdotV, vec3 F0)
{
    if(dirLightData.strength == 0.0)
    {
        return vec3(0.0);
    }
    vec3 L = normalize(-dirLightData.direction);
    vec3 H = normalize(V + L);
    float NdotL = max(dot(N, L), 0.0);
    float cosTheta = clamp(dot(H, V), 0.0, 1.0);
    vec3 F = fresnelSchlick(cosTheta, F0);
    float D = DistributionGGX(N, H, roughness);
    float G = GeometrySmith(N, V, L, roughness);
    vec3 specular = (F*D*G) / max(4.0 * NdotV * NdotL, 0.00001);
    vec3 kD = (vec3(1.0) - specular) * (1.0 - metallic);
    vec3 diffuse = kD * albedo / PI;
    return (diffuse + specular) * dirLightData.color * dirLightData.strength * NdotL;
}

vec3 getSpotLightColor(vec3 N, vec3 V, float NdotV, vec3 F0)
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
    float cosTheta = max(dot(H, V), 0.0);
    vec3 F = fresnelSchlick(cosTheta, F0);
    float D = DistributionGGX(N, H, roughness);
    float G = GeometrySmith(N, V, L, roughness);
    float dist = length(spotLightData.position - fs_in.pos);
    float attenuation = 1.0 / (dist * dist);
    vec3 specular = (F*D*G) / max(4.0 * NdotV * NdotL, 0.00001);
    vec3 kD = (vec3(1.0) - specular) * (1.0 - metallic);
    vec3 diffuse = kD * albedo / PI;
    return (diffuse + specular) * spotLightData.color * spotLightData.strength * attenuation * intensity * NdotL;
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