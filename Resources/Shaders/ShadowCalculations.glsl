// reference: https://maxest.gct-game.net/content/chss.pdf
// https://github.com/maxest/MaxestFramework/blob/5b06324aea21227fbbebd3257d41b75f76135578/samples/shadows/data/common.hlsl
// https://github.com/maxest/MaxestFramework/blob/5b06324aea21227fbbebd3257d41b75f76135578/samples/shadows/data/shadow_mask_ps.hlsl

//SHADOW>include UboWindow.glsl

//SHADOW>include VOGEL_DISK

// Provided vogel disks are arrays of vec2.
// Each vec2 consists of radius and angle (theta).

vec2 sampleShadowVogelDisk(int sampleIndex, float phi)
{
    vec2 rTheta = shadowVogelDisk[sampleIndex];
    rTheta.g += phi;
    return vec2(rTheta.r * cos(rTheta.g), rTheta.r * sin(rTheta.g));
}

vec2 samplePenumbraVogelDisk(int sampleIndex, float phi)
{
    vec2 rTheta = penumbraVogelDisk[sampleIndex];
    rTheta.g += phi;
    return vec2(rTheta.r * cos(rTheta.g), rTheta.r * sin(rTheta.g));
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
    for(int i = 0; i < VOGEL_PS; ++i)
    {
        float depth = texture(text,
        texCoords + samplePenumbraVogelDisk(
            i, texture(ignTexture, gl_FragCoord.xy / windowSize).x) * searchWidth).r;
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
    vec2 screenCoords = gl_FragCoord.xy / windowSize;
    vec3 projCoords = (lightSpacePos.xyz / lightSpacePos.w) * 0.5 + 0.5;
    if(projCoords.z > 1.0)
    {
        return 0.0;
    }
    float penumbraRatio = texture(penumbraText, screenCoords).r;
    if(penumbraRatio == 0.0)
    {
        return 0.0;
    }
    float filterRadiusUV = penumbraRatio * lightSize * nearZ / projCoords.z;
    float shadow = 0.0;
    for(int i = 0; i < VOGEL_SS; ++i)
    {
        float depth = texture(text,
        projCoords.xy + sampleShadowVogelDisk(
            i, texture(ignTexture, screenCoords).x) * filterRadiusUV).r;
        if(depth < projCoords.z - 0.008)
        {
            shadow += 1.0;
        }
    }
    shadow /= VOGEL_SS;
    return shadow;
}
