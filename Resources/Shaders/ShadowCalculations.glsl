#define PCF_KERNEL_SIZE 3
const int PCF_MAX = PCF_KERNEL_SIZE / 2, PCF_MIN = -PCF_MAX;
const float PCF_KERNEL_SQUARED = PCF_KERNEL_SIZE*PCF_KERNEL_SIZE;
const float TEX_SIZE_MULTIPLIER = 1.0f;

float calcShadow(float worldNdotL, vec4 lightSpacePos, sampler2D text)
{
    vec3 projCoords = (lightSpacePos.xyz / lightSpacePos.w) * 0.5 + 0.5;
    if(projCoords.z <= 1.0)
    {
        float closestDepth = texture(text, projCoords.xy).r;
        float currentDepth = projCoords.z;
        float bias = max(0.005 * (1.0 - worldNdotL), 0.00125);
        if(currentDepth - bias > closestDepth)
        {
            return 0.0;
        }
        vec2 texelSize = TEX_SIZE_MULTIPLIER/textureSize(text, 0);
        float shadow = 0.0;
        for(int x=PCF_MIN;x<=PCF_MAX;++x)
        {
            for(int y=PCF_MIN;y<=PCF_MAX;++y)
            {
                float pcfDepth = texture(text, projCoords.xy + vec2(x,y)*texelSize).r;
                shadow += currentDepth  - bias > pcfDepth ? 0.0 : 1.0;
            }
        }
        return (shadow/PCF_KERNEL_SQUARED);
    }
    return 1.0;
}