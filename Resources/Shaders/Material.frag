#version 430 core

//SHADOW>include UboMaterial.glsl

//SHADOW>include LightStructs.glsl

//SHADOW>include UboLights.glsl

layout(binding = 10) uniform sampler2D directionalShadow;
layout(binding = 11) uniform sampler2D directionalPenumbra;
layout(binding = 12) uniform sampler2D spotShadow;
layout(binding = 13) uniform sampler2D spotPenumbra;

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

//SHADOW>include PBRFunctions.glsl

//SHADOW>include ShadowCalculations.glsl

vec3 getDirectionalLightColor(vec3 N, vec3 V, float NdotV, vec3 F0)
{
    if(dirLightData.strength == 0.0)
    {
        return vec3(0.0);
    }
    vec3 L = normalize(-dirLightData.direction);
    float NdotL = max(dot(N, L), 0.0);
    float shadow = calcShadow(NdotL, fs_in.dirSpacePos, dirLightData.nearZ, dirLightData.lightSize, directionalShadow, directionalPenumbra);
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
    float shadow = calcShadow(NdotL, fs_in.spotSpacePos, spotLightData.nearZ, spotLightData.lightSize, spotShadow, spotPenumbra);
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
