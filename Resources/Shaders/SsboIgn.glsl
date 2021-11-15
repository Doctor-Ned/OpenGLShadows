layout (std430, binding = 4) readonly buffer InterleavedGradientNoise
{
    uvec2 ignDimensions;
    float interleavedGradientNoise[];
};