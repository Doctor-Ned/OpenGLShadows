#version 430 core
layout (location = 0) in vec3 pos;
layout (location = 1) in vec3 normal;

layout (std140, binding = 0) uniform ModelViewProjection
{
    mat4 model;
    mat4 view;
    mat4 projection;
    vec3 viewPosition;
    float paddingMvp;
};

out VS_OUT
{
    vec3 pos;
    vec3 normal;
    vec3 viewPosition;
    vec3 toView;
} vs_out;

void main()
{
    vs_out.pos = vec3(model * vec4(pos, 1.0));
    vs_out.normal = normalize(transpose(inverse(mat3(model))) * normal);
    vs_out.viewPosition = viewPosition;
    vs_out.toView = normalize(viewPosition - vs_out.pos);
    gl_Position = projection * view * vec4(vs_out.pos, 1.0);
}