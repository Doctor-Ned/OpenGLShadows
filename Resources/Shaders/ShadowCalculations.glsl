//SHADOW>include ShadowVariants.glsl

//SHADOW>include UboWindow.glsl

#if SHADOW_MASTER || SHADOW_CHSS
//SHADOW>include VOGEL_DISK

#if SHADOW_MASTER
//SHADOW>include SsboIgn.glsl
// Provided vogel disks are arrays of vec2.
// Each vec2 consists of radius and angle (theta).

float sampleInterleavedGradientNoise()
{
    return interleavedGradientNoise[int(gl_FragCoord.x) * ignDimensions.x + int(gl_FragCoord.y)];
}

vec2 sampleShadowVogelDisk(int sampleIndex)
{
    vec2 rTheta = shadowVogelDisk[sampleIndex];
    rTheta.g += sampleInterleavedGradientNoise();
    return vec2(rTheta.r * cos(rTheta.g), rTheta.r * sin(rTheta.g));
}

vec2 samplePenumbraVogelDisk(int sampleIndex)
{
    vec2 rTheta = penumbraVogelDisk[sampleIndex];
    rTheta.g += sampleInterleavedGradientNoise();
    return vec2(rTheta.r * cos(rTheta.g), rTheta.r * sin(rTheta.g));
}
#else
const float VOGEL_SS_SQRT = sqrt(VOGEL_SS);
const float VOGEL_PS_SQRT = sqrt(VOGEL_PS);

vec2 sampleShadowVogelDisk(int sampleIndex, float phi)
{
    float r = sqrt(sampleIndex + 0.5f) / VOGEL_SS_SQRT;
    float theta = sampleIndex * 2.4f + phi;
    return vec2(r * cos(theta), r * sin(theta));
}

vec2 samplePenumbraVogelDisk(int sampleIndex, float phi)
{
    float r = sqrt(sampleIndex + 0.5f) / VOGEL_PS_SQRT;
    float theta = sampleIndex * 2.4f + phi;
    return vec2(r * cos(theta), r * sin(theta));
}

float interleavedGradientNoise(vec2 screenPos)
{
    const vec3 factors = vec3(0.06711056f, 0.00583715f, 52.9829189f);
    return fract(factors.z * sin(dot(screenPos.xy, factors.xy)));
}
#endif

float penumbraSize(float receiverDepth, float blockerDepth)
{
    return (receiverDepth-blockerDepth) / blockerDepth;
}

#if SHADOW_MASTER
float calcPenumbra(vec4 lightSpacePos, float nearZ, float lightSize, sampler2D text)
{
    vec3 projCoords = (lightSpacePos.xyz / lightSpacePos.w) * 0.5 + 0.5;
    if(projCoords.z > 1.0)
    {
        return 0.0;
    }
    float blockerDepth = 0.0;
    int numBlockers = 0;
    vec2 texCoords = projCoords.xy;
    float searchWidth = lightSize * (projCoords.z - nearZ) / projCoords.z;
    for(int i = 0; i < VOGEL_PS; ++i)
    {
        float depth = texture(text, texCoords + samplePenumbraVogelDisk(i) * searchWidth).r;
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
        float depth = texture(text, projCoords.xy + sampleShadowVogelDisk(i) * filterRadiusUV).r;
        if(depth < projCoords.z - 0.008)
        {
            shadow += 1.0;
        }
    }
    shadow /= VOGEL_SS;
    return shadow;
}
#else
float calcPenumbra(vec4 lightSpacePos, float nearZ, float lightSize, sampler2D text)
{
    vec3 projCoords = (lightSpacePos.xyz / lightSpacePos.w) * 0.5 + 0.5;
    if(projCoords.z > 1.0)
    {
        return 0.0;
    }
    float blockerDepth = 0.0;
    int numBlockers = 0;
    vec2 texCoords = projCoords.xy;
    float searchWidth = lightSize * (projCoords.z - nearZ) / projCoords.z;
    for(int i = 0; i < VOGEL_PS; ++i)
    {
        float depth = texture(text, texCoords + samplePenumbraVogelDisk(i, interleavedGradientNoise(gl_FragCoord.xy / windowSize)) * searchWidth).r;
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
    for(int i = 0; i < VOGEL_SS; ++i)
    {
        float depth = texture(text, projCoords.xy + sampleShadowVogelDisk(i, interleavedGradientNoise(gl_FragCoord.xy / windowSize)) * filterRadiusUV).r;
        if(depth < projCoords.z - 0.008)
        {
            shadow += 1.0;
        }
    }
    shadow /= VOGEL_SS;
    return shadow;
}
#endif
#elif SHADOW_PCSS

//SHADOW>include POISSON

#if PCSS_BLOCKERS <= 0 || PCSS_BLOCKERS > PCSS_MAX_BLOCKERS
#error Invalid PCSS_BLOCKERS value!
#endif

#if PCSS_FILTER_SIZE <= 0 || PCSS_FILTER_SIZE > PCSS_MAX_BLOCKERS
#error Invalid PCSS_FILTER_SIZE value!
#endif

#if PCSS_MAX_BLOCKERS == 32
vec2 POISSON_DISK[PCSS_MAX_BLOCKERS] = {
    vec2(0.06407013, 0.05409927),
    vec2(0.7366577, 0.5789394),
    vec2(-0.6270542, -0.5320278),
    vec2(-0.4096107, 0.8411095),
    vec2(0.6849564, -0.4990818),
    vec2(-0.874181, -0.04579735),
    vec2(0.9989998, 0.0009880066),
    vec2(-0.004920578, -0.9151649),
    vec2(0.1805763, 0.9747483),
    vec2(-0.2138451, 0.2635818),
    vec2(0.109845, 0.3884785),
    vec2(0.06876755, -0.3581074),
    vec2(0.374073, -0.7661266),
    vec2(0.3079132, -0.1216763),
    vec2(-0.3794335, -0.8271583),
    vec2(-0.203878, -0.07715034),
    vec2(0.5912697, 0.1469799),
    vec2(-0.88069, 0.3031784),
    vec2(0.5040108, 0.8283722),
    vec2(-0.5844124, 0.5494877),
    vec2(0.6017799, -0.1726654),
    vec2(-0.5554981, 0.1559997),
    vec2(-0.3016369, -0.3900928),
    vec2(-0.5550632, -0.1723762),
    vec2(0.925029, 0.2995041),
    vec2(-0.2473137, 0.5538505),
    vec2(0.9183037, -0.2862392),
    vec2(0.2469421, 0.6718712),
    vec2(0.3916397, -0.4328209),
    vec2(-0.03576927, -0.6220032),
    vec2(-0.04661255, 0.7995201),
    vec2(0.4402924, 0.3640312),
};
#elif PCSS_MAX_BLOCKERS == 16
vec2 POISSON_DISK[PCSS_MAX_BLOCKERS] =
{
    vec2(-0.94201624, -0.39906216),
    vec2(0.94558609, -0.76890725),
    vec2(-0.094184101, -0.92938870),
    vec2(0.34495938, 0.29387760),
    vec2(-0.91588581, 0.45771432),
    vec2(-0.81544232, -0.87912464),
    vec2(-0.38277543, 0.27676845),
    vec2(0.97484398, 0.75648379),
    vec2(0.44323325, -0.97511554),
    vec2(0.53742981, -0.47373420),
    vec2(-0.26496911, -0.41893023),
    vec2(0.79197514, 0.19090188),
    vec2(-0.24188840, 0.99706507),
    vec2(-0.81409955, 0.91437590),
    vec2(0.19984126, 0.78641367),
    vec2(0.14383161, -0.14100790)
};
#else
#error Invalid PCSS_MAX_BLOCKERS value!
#endif

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
#elif SHADOW_PCF
//SHADOW>include FILTER_SIZE
const int PCF_MAX = FILTER_SIZE / 2, PCF_MIN = -PCF_MAX;
const float FILTER_SIZE_SQUARED = FILTER_SIZE * FILTER_SIZE;
const float TEX_SIZE_MULTIPLIER = 1.0f;
float calcShadow(float worldNdotL, vec4 lightSpacePos, sampler2D text)
{
    vec3 projCoords = (lightSpacePos.xyz / lightSpacePos.w) * 0.5 + 0.5;
    if(projCoords.z  <= 1.0)
    {
        float closestDepth = texture(text, projCoords.xy).r;
        float currentDepth = projCoords.z;
        float bias = max(0.007 * (1.0 - worldNdotL), 0.0085);
        if(currentDepth - bias > closestDepth)
        {
            return 1.0;
        }
        vec2 texelSize = TEX_SIZE_MULTIPLIER / textureSize(text, 0);
        float shadow = 0.0;
        for(int x = PCF_MIN; x <= PCF_MAX; ++x)
        {
            for(int y = PCF_MIN; y <= PCF_MAX; ++y)
            {
                float pcfDepth = texture(text, projCoords.xy + vec2(x,y) * texelSize).r;
                shadow += currentDepth - bias > pcfDepth ? 0.0 : 1.0;
            }
        }
        return 1.0 - (shadow / FILTER_SIZE_SQUARED);
    }
    return 0.0;
}
#elif SHADOW_VSM
float linstep(float low, float high, float v)
{
    return clamp((v-low)/(high-low), 0.0, 1.0);
}
float calcShadow(vec4 lightSpacePos, sampler2D text)
{
    vec3 projCoords = (lightSpacePos.xyz / lightSpacePos.w) * 0.5 + 0.5;
    if(projCoords.z > 0.999)
    {
        return 0.0;
    }
    vec2 moments = texture(text, projCoords.xy).rg;
    float p = step(projCoords.z, moments.x);
    float variance = max(moments.y - moments.x * moments.x, 0.0002);
    float d = projCoords.z - moments.x;
    float pMax = linstep(0.25, 1.0, variance / (variance + d*d));
    return 1.0 - min(max(p, pMax), 1.0);
}
#elif SHADOW_BASIC
float calcShadow(float worldNdotL, vec4 lightSpacePos, sampler2D text)
{
    vec3 projCoords = (lightSpacePos.xyz / lightSpacePos.w) * 0.5 + 0.5;
    if(projCoords.z  <= 1.0)
    {
        float closestDepth = texture(text, projCoords.xy).r;
        float currentDepth = projCoords.z;
        float bias = max(0.005 * (1.0 - worldNdotL), 0.0009);
        if(currentDepth - bias > closestDepth)
        {
            return 1.0;
        }
    }
    return 0.0;
}
#endif
