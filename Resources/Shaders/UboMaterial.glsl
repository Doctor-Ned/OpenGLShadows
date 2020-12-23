layout (std140, binding = 1) uniform Material
{
    vec3 albedo;
    float roughness;
    vec3 paddingM;
    float metallic;
};