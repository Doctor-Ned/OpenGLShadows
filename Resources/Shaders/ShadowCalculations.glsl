// reference: https://maxest.gct-game.net/content/chss.pdf
// https://github.com/maxest/MaxestFramework/blob/5b06324aea21227fbbebd3257d41b75f76135578/samples/shadows/data/common.hlsl
// https://github.com/maxest/MaxestFramework/blob/5b06324aea21227fbbebd3257d41b75f76135578/samples/shadows/data/shadow_mask_ps.hlsl

#define VOGEL_SAMPLES_COUNT 32

const float VOGEL_SAMPLES_COUNT_SQRT = sqrt(VOGEL_SAMPLES_COUNT);

vec2 sampleVogelDisk(int sampleIndex, float phi)
{
    float r = sqrt(sampleIndex + 0.5f) / VOGEL_SAMPLES_COUNT_SQRT;
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

float calcShadow(float worldNdotL, vec4 lightSpacePos, float nearZ, float lightSize, sampler2D text)
{
    vec3 projCoords = (lightSpacePos.xyz / lightSpacePos.w) * 0.5 + 0.5;
    if(projCoords.z > 1.0)
    {
        return 0.0;
    }
    float blockerDepth = 0.0;
    int numBlockers = 0;
    vec2 texCoords=projCoords.xy;
    float searchWidth = lightSize * (projCoords.z - nearZ) / projCoords.z;
    for(int i=0;i<PCSS_BLOCKERS;++i)
    {
        float depth = texture(text, texCoords + POISSON_DISK[i] * searchWidth).r;
        if(depth < projCoords.z)
        {
            blockerDepth += depth;
            ++numBlockers;
        }
    }
    if(numBlockers == 0)
    {
        return 0.0;
    }
    blockerDepth /= numBlockers;
    float penumbraRatio = (projCoords.z - blockerDepth) / blockerDepth;
    float filterRadiusUV = penumbraRatio * lightSize * nearZ / projCoords.z;
    float shadow = 0.0;
    for(int i=0;i<PCSS_FILTER_SIZE;++i)
    {
        float depth = texture(text, projCoords.xy + POISSON_DISK[i] * filterRadiusUV).r;
        if(depth < projCoords.z - 0.008)
        {
            shadow += 1.0;
        }
    }
    shadow /= PCSS_FILTER_SIZE;
    return shadow;
}