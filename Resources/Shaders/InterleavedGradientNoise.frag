#version 430 core

//SHADOW>include UboWindow.glsl

out float outColor;

void main()
{
    const vec3 factors = vec3(0.06711056f, 0.00583715f, 52.9829189f);
    outColor = fract(factors.z * sin(dot(gl_FragCoord.xy / windowSize, factors.xy)));
}
