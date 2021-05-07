// reference: https://maxest.gct-game.net/content/chss.pdf
// https://github.com/maxest/MaxestFramework/blob/5b06324aea21227fbbebd3257d41b75f76135578/samples/shadows/data/common.hlsl
// https://github.com/maxest/MaxestFramework/blob/5b06324aea21227fbbebd3257d41b75f76135578/samples/shadows/data/shadow_mask_ps.hlsl

//SHADOW>includedfrom UboWindow.glsl
layout (std140, binding = 3) uniform Window
{
    vec2 windowSize;
};
//SHADOW>endinclude UboWindow.glsl

#define VOGEL_SAMPLES_COUNT 32
#define VOGEL_PENUMBRA_SAMPLES_COUNT 16

const float VOGEL_SAMPLES_COUNT_SQRT = sqrt(VOGEL_SAMPLES_COUNT);
const float VOGEL_PENUMBRA_SAMPLES_COUNT_SQRT = sqrt(VOGEL_PENUMBRA_SAMPLES_COUNT);

vec2 sampleVogelDisk(int sampleIndex, float phi)
{
    float r = sqrt(sampleIndex + 0.5f) / VOGEL_SAMPLES_COUNT_SQRT;
    float theta = sampleIndex * 2.4f + phi;
    return vec2(r * cos(theta), r * sin(theta));
}
vec2 sampleVogelPenumbraDisk(int sampleIndex, float phi)
{
    float r = sqrt(sampleIndex + 0.5f) / VOGEL_PENUMBRA_SAMPLES_COUNT_SQRT;
    float theta = sampleIndex * 2.4f + phi;
    return vec2(r * cos(theta), r * sin(theta));
}

float interleavedGradientNoise(vec2 screenPos)
{
    const vec3 factors = vec3(0.06711056f, 0.00583715f, 52.9829189f);
    return fract(factors.z * sin(dot(screenPos.xy, factors.xy)));
}

float penumbraSize(float receiverDepth, float blockerDepth)
{
    return (receiverDepth-blockerDepth) / blockerDepth;
}

vec2 calcPenumbra(vec4 lightSpacePos, float nearZ, float lightSize, sampler2D text)
{
    vec3 projCoords = (lightSpacePos.xyz / lightSpacePos.w) * 0.5 + 0.5;
    if(projCoords.z > 1.0)
    {
        return vec2(0.0, 0.0);
    }
    float blockerDepth = 0.0;
    int numBlockers = 0;
    vec2 texCoords=projCoords.xy;
    float searchWidth = lightSize * (projCoords.z - nearZ) / projCoords.z;
    for(int i = 0; i < VOGEL_PENUMBRA_SAMPLES_COUNT; ++i)
    {
        float depth = texture(text,
        texCoords + sampleVogelPenumbraDisk(
            i, interleavedGradientNoise(gl_FragCoord.xy / windowSize)) * searchWidth).r;
        if(depth < projCoords.z)
        {
            blockerDepth += depth;
            ++numBlockers;
        }
    }
    if(numBlockers == 0)
    {
        return vec2(0.0, 0.0);
    }
    blockerDepth /= numBlockers;
    return vec2((projCoords.z - blockerDepth) / blockerDepth, 1.0);
}

float calcShadow(float worldNdotL, vec4 lightSpacePos, float nearZ, float lightSize, sampler2D text, sampler2D penumbraText)
{
    vec3 projCoords = (lightSpacePos.xyz / lightSpacePos.w) * 0.5 + 0.5;
    if(projCoords.z > 1.0)
    {
        return 0.0;
    }
    vec2 penumbraRatio = texture(penumbraText, gl_FragCoord.xy / windowSize).rg;
    if(penumbraRatio.g == 0.0)
    {
        return 0.0;
    }
    float filterRadiusUV = penumbraRatio.r * lightSize * nearZ / projCoords.z;
    float shadow = 0.0;
    for(int i = 0; i < VOGEL_SAMPLES_COUNT; ++i)
    {
        float depth = texture(text,
        projCoords.xy + sampleVogelDisk(
            i, interleavedGradientNoise(gl_FragCoord.xy / windowSize)) * filterRadiusUV).r;
        if(depth < projCoords.z - 0.008)
        {
            shadow += 1.0;
        }
    }
    shadow /= VOGEL_SAMPLES_COUNT;
    return shadow;
}
