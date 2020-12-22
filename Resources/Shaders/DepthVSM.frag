#version 430 core

out vec4 outColor;

void main()
{
    // reference: https://github.com/BennyQBD/3DEngineCpp/blob/001bc572e85e090df77cd9de2f3cfa61ab3b563a/3DEngineCpp/res/shaders/sampling.glh
    float depth = gl_FragCoord.z;
    float dx = dFdx(depth);
    float dy = dFdy(depth);
    float moment2 = depth * depth * 0.25 * (dx*dx+dy*dy);
    outColor = vec4(depth, moment2, 0.0, 0.0);
}