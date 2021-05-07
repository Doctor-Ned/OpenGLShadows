#version 430 core

//SHADOW>includedfrom LightStructs.glsl
struct DirectionalLightData
{
    mat4 lightSpace;
    vec3 color;
    float strength;
    vec3 direction;
    float nearZ;
    vec2 padding;
    float farZ;
    float lightSize;
};

struct SpotLightData
{
    mat4 lightSpace;
    vec3 color;
    float strength;
    vec3 direction;
    float innerCutOff;
    vec3 position;
    float outerCutOff;
    float nearZ;
    float farZ;
    float lightSize;
    float padding;
};
//SHADOW>endinclude LightStructs.glsl

//SHADOW>includedfrom UboLights.glsl
layout (std140, binding = 2) uniform Lights
{
    DirectionalLightData dirLightData;
    SpotLightData spotLightData;
    vec3 paddingL;
    float ambient;
};
//SHADOW>endinclude UboLights.glsl

layout(binding = 12) uniform sampler2D spotShadow;

in VS_OUT
{
    vec4 spotSpacePos;
} fs_in;

out vec4 outColor;

//SHADOW>includedfrom ShadowCalculations.glsl
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

float calcPenumbra(vec4 lightSpacePos, float nearZ, float lightSize, sampler2D text)
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
        return 0.0;
    }
    blockerDepth /= numBlockers;
    return (projCoords.z - blockerDepth) / blockerDepth;
}

float calcShadow(float worldNdotL, vec4 lightSpacePos, float nearZ, float lightSize, sampler2D text, sampler2D penumbraText)
{
    vec3 projCoords = (lightSpacePos.xyz / lightSpacePos.w) * 0.5 + 0.5;
    if(projCoords.z > 1.0)
    {
        return 0.0;
    }
    float penumbraRatio = texture(penumbraText, gl_FragCoord.xy / windowSize).r;
    if(penumbraRatio == 0.0)
    {
        return 0.0;
    }
    float filterRadiusUV = penumbraRatio * lightSize * nearZ / projCoords.z;
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

//SHADOW>endinclude ShadowCalculations.glsl

void main()
{
    outColor = vec4(calcPenumbra(fs_in.spotSpacePos, spotLightData.nearZ, spotLightData.lightSize, spotShadow), 0.0, 0.0, 1.0);
}
