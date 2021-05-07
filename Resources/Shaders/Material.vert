#version 430 core
layout (location = 0) in vec3 pos;
layout (location = 1) in vec3 normal;

//SHADOW>include UboMvp.glsl

//SHADOW>include LightStructs.glsl

//SHADOW>include UboLights.glsl

out VS_OUT
{
    vec3 pos;
    vec3 normal;
    vec3 viewPosition;
    vec3 toView;
    vec4 dirSpacePos;
    vec4 spotSpacePos;
} vs_out;

void main()
{
    vs_out.pos = vec3(model * vec4(pos, 1.0));
    vs_out.normal = normalize(transpose(inverse(mat3(model))) * normal);
    vs_out.viewPosition = viewPosition;
    vs_out.toView = normalize(viewPosition - vs_out.pos);
    vs_out.dirSpacePos = dirLightData.lightSpace * vec4(vs_out.pos, 1.0);
    vs_out.spotSpacePos = spotLightData.lightSpace * vec4(vs_out.pos, 1.0);
    gl_Position = projection * view * vec4(vs_out.pos, 1.0);
}
