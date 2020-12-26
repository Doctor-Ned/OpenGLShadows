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
    float variance = max(moments.y - moments.x*moments.x, 0.0002);
    float d = projCoords.z - moments.x;
    float pMax = linstep(0.25, 1.0, variance / (variance + d*d));
    return min(max(p, pMax), 1.0);
}