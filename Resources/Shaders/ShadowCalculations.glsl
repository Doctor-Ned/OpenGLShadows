float calcShadow(float worldNdotL, vec4 lightSpacePos, sampler2D text)
{
    vec3 projCoords = (lightSpacePos.xyz / lightSpacePos.w) * 0.5 + 0.5;
    if(projCoords.z <= 1.0)
    {
        float closestDepth = texture(text, projCoords.xy).r;
        float currentDepth = projCoords.z;
        float bias = max(0.005 * (1.0 - worldNdotL), 0.0009);
        if(currentDepth - bias > closestDepth)
        {
            return 0.0;
        }
    }
    return 1.0;
}