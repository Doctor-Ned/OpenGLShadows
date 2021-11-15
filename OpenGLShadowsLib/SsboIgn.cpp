#include "SsboIgn.h"

#include <glm/gtc/type_ptr.hpp>

shadow::SsboIgn::SsboIgn(GLsizei windowWidth, GLsizei windowHeight)
    : ShaderStorageBufferObject("InterleavedGradientNoise", 4, sizeof(glm::uvec2) + (sizeof(float) * windowWidth * windowHeight)) {}

shadow::SsboIgn::~SsboIgn()
{
    dataStruct.ignDimensions = {};
    delete[] dataStruct.interleavedGradientNoise;
}

void shadow::SsboIgn::set(SsboIgnStruct& data)
{
    assert(data.ignDimensions[0] > 0);
    assert(data.ignDimensions[1] > 0);
    assert(data.interleavedGradientNoise);
    dataStruct.ignDimensions = data.ignDimensions;
    bufferSubData(&dataStruct, sizeof(glm::uvec2), offsetof(SsboIgnStruct, ignDimensions));
    delete[] dataStruct.interleavedGradientNoise;
    size_t dataCount = dataStruct.ignDimensions[0] * dataStruct.ignDimensions[1];
    dataStruct.interleavedGradientNoise = new float[dataCount];
    memcpy(dataStruct.interleavedGradientNoise, data.interleavedGradientNoise, sizeof(float) * dataCount);
    bufferSubData(&dataStruct, sizeof(float) * dataCount, offsetof(SsboIgnStruct, interleavedGradientNoise));
}

void shadow::SsboIgn::resize(GLsizei windowWidth, GLsizei windowHeight)
{
    assert(windowWidth > 0);
    assert(windowHeight > 0);
    SHADOW_DEBUG("Regenerating interleaved {}x{} ({}) gradient noise samples...", windowWidth, windowHeight, windowWidth * windowHeight);
    dataStruct.ignDimensions = glm::uvec2(windowWidth, windowHeight);
    bufferSubData(&dataStruct.ignDimensions, sizeof(glm::uvec2), offsetof(SsboIgnStruct, ignDimensions));
    delete[] dataStruct.interleavedGradientNoise;
    dataStruct.interleavedGradientNoise = new float[dataStruct.ignDimensions[0] * dataStruct.ignDimensions[1]];
    int counter = -1;
    for (int x = 0; x < windowWidth; ++x)
    {
        for (int y = 0; y < windowHeight; ++y)
        {
            dataStruct.interleavedGradientNoise[++counter] = getInterleavedGradientNoise(x, y, windowWidth, windowHeight);
        }
    }
    bufferSubData(dataStruct.interleavedGradientNoise, sizeof(float) * windowWidth * windowHeight, offsetof(SsboIgnStruct, interleavedGradientNoise));
    SHADOW_DEBUG("Interleaved gradient noise regenerated!");
}

float shadow::SsboIgn::getInterleavedGradientNoise(GLsizei x, GLsizei y, GLsizei windowWidth, GLsizei windowHeight) const
{
    static const glm::vec3 FACTORS{ 0.06711056f, 0.00583715f, 52.9829189f };
    glm::vec2 uv = glm::vec2(x, y) / glm::vec2(windowWidth, windowHeight);
    float noise = FACTORS.z * std::sinf(dot(uv, glm::vec2(FACTORS.x, FACTORS.y)));
    return noise - static_cast<float>(static_cast<long>(noise));
}
