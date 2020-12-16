#include "Primitives.h"
#include "ShadowUtils.h"

std::shared_ptr<shadow::PrimitiveData> shadow::Primitives::cuboid()
{
    return cuboid(1.0f, 1.0f, 1.0f);
}

std::shared_ptr<shadow::PrimitiveData> shadow::Primitives::cuboid(float width, float height, float length)
{
    float hWidth = width * 0.5f, hHeight = height * 0.5f, hLength = length * 0.5f;
    std::vector<glm::vec3> vertices{
        { -hWidth, -hHeight, hLength },
        { hWidth, -hHeight, hLength },
        { hWidth, -hHeight, -hLength },
        { -hWidth, -hHeight, -hLength },
        { -hWidth, hHeight, hLength },
        { hWidth, hHeight, hLength },
        { hWidth, hHeight, -hLength },
        { -hWidth, hHeight, -hLength }
    };
    // the textures are nasty, maybe i should make all the faces separate for that *thinking emoji*
    std::vector<glm::vec2> texCoords{
        { 0.0f, 0.0f },
        { 1.0f, 0.0f },
        { 1.0f, 0.0f },
        { 0.0f, 0.0f },
        { 0.0f, 1.0f },
        { 1.0f, 1.0f },
        { 1.0f, 1.0f },
        { 1.0f, 0.0f }
    };
    std::vector<GLuint> indices{ 0,1,5,0,5,4,1,2,6,1,6,5,2,3,7,2,7,6,3,0,4,3,4,7,1,0,3,1,3,2,4,5,6,4,6,7 };
    return std::make_shared<PrimitiveData>(vertices, texCoords, indices);
}

std::shared_ptr<shadow::PrimitiveData> shadow::Primitives::cube()
{
    return cube(1.0f);
}

std::shared_ptr<shadow::PrimitiveData> shadow::Primitives::cube(float size)
{
    return cuboid(size, size, size);
}

std::shared_ptr<shadow::PrimitiveData> shadow::Primitives::cone(unsigned int precision)
{
    return cone(precision, 1.0f, 1.0f);
}

std::shared_ptr<shadow::PrimitiveData> shadow::Primitives::cone(unsigned int precision, float radius, float height)
{
    std::vector<glm::vec3> vertices = createHorizontalCircle(precision, radius, 0.0f, 0.0f, 0.0f);
    if (vertices.empty())
    {
        return {};
    }
    std::vector<glm::vec2> texCoords{};
    std::vector<GLuint> indices{};
    for (unsigned int i = 0; i < vertices.size(); ++i)
    {
        texCoords.emplace_back(i % 2 == 0 ? 0.0f : 1.0f, 0.0f);
    }
    vertices.emplace_back(0.0f, 0.0f, 0.0f);
    vertices.emplace_back(0.0f, height, 0.0f);
    texCoords.emplace_back(0.5f, 1.0f);
    texCoords.emplace_back(0.5f, 1.0f);
    unsigned int bottomIndex = precision, topIndex = bottomIndex + 1U;
    for (unsigned int i = 0; i < precision; ++i)
    {
        unsigned int nextIndex = (i + 1U) % precision;
        indices.insert(indices.end(), { i,nextIndex,topIndex,nextIndex,i,bottomIndex });
    }
    return std::make_shared<PrimitiveData>(vertices, texCoords, indices);
}

std::shared_ptr<shadow::PrimitiveData> shadow::Primitives::cylinder(unsigned int precision, unsigned int heightSegments)
{
    return cylinder(precision, heightSegments, 1.0f, 1.0f);
}

std::shared_ptr<shadow::PrimitiveData> shadow::Primitives::cylinder(unsigned int precision, unsigned int heightSegments, float radius, float height)
{
    std::vector<glm::vec3> circle = createHorizontalCircle(precision, radius, 0.0f, 0.0f, 0.0f);
    if (circle.empty())
    {
        return {};
    }
    std::vector<glm::vec3> vertices{};
    std::vector<glm::vec2> texCoords{};
    std::vector<GLuint> indices{};
    const float heightStep = height / static_cast<float>(heightSegments);
    for (unsigned int i = 0; i < heightSegments + 1U; ++i)
    {
        float texY = i % 2 == 0 ? 0.0f : 1.0f;
        for (unsigned int j = 0; j < circle.size(); ++j)
        {
            float texX = j % 2 == 0 ? 0.0f : 1.0f;
            glm::vec3 current = circle[j];
            current[1] += heightStep * static_cast<float>(i);
            vertices.push_back(current);
            texCoords.emplace_back(texX, texY);
        }
    }
    unsigned int bottomIndex = static_cast<unsigned int>(vertices.size()), topIndex = bottomIndex + 1U;
    vertices.emplace_back(0.0f, 0.0f, 0.0f);
    vertices.emplace_back(0.0f, height, 0.0f);
    texCoords.emplace_back(0.5f, 1.0f);
    texCoords.emplace_back(0.5f, 1.0f);
    unsigned int topCircleBegin = precision * heightSegments;
    for (unsigned int i = 0; i < precision; ++i)
    {
        unsigned int nextIndex = (i + 1U) % precision;
        indices.insert(indices.end(), { nextIndex,i,bottomIndex,i + topCircleBegin,nextIndex + topCircleBegin,topIndex });
    }
    for (unsigned int i = 0; i < heightSegments; ++i)
    {
        for (unsigned int j = 0; j < precision; ++j)
        {
            unsigned int currIndex = i * precision + j;
            unsigned int nextIndex = i * precision + (j + 1U) % precision;
            unsigned int upper = currIndex + precision;
            unsigned int upperNext = nextIndex + precision;
            indices.insert(indices.end(), { currIndex,nextIndex,upperNext,currIndex,upperNext,upper });
        }
    }
    return std::make_shared<PrimitiveData>(vertices, texCoords, indices);
}

std::shared_ptr<shadow::PrimitiveData> shadow::Primitives::plane()
{
    return plane(1.0f, 1.0f, glm::vec2(1.0f, 1.0f));
}

std::shared_ptr<shadow::PrimitiveData> shadow::Primitives::plane(float width, float length, glm::vec2 texCoordsMax)
{
    float hWidth = width * 0.5f, hLength = length * 0.5f;
    std::vector<glm::vec3> vertices{
        { -hWidth, 0.0f, hLength },
        { hWidth, 0.0f, hLength },
        { hWidth, 0.0f, -hLength },
        { -hWidth, 0.0f, -hLength }
    };
    std::vector<glm::vec2> texCoords{
        { 0.0f, 0.0f },
        { texCoordsMax.x, 0.0f },
        texCoordsMax,
        { 0.0f, texCoordsMax.y }
    };
    std::vector<GLuint> indices{ 2,0,1,3,0,2 };
    return std::make_shared<PrimitiveData>(vertices, texCoords, indices);
}

std::shared_ptr<shadow::PrimitiveData> shadow::Primitives::sphere(unsigned int precision)
{
    return sphere(precision, 1.0f);
}

std::shared_ptr<shadow::PrimitiveData> shadow::Primitives::sphere(unsigned int precision, float radius)
{
    std::vector<glm::vec3> vertices{};
    std::vector<glm::vec2> texCoords{};
    std::vector<GLuint> indices{};
    const float heightStep = radius * 2.0f / static_cast<float>(precision - 1U);
    for (unsigned int i = 1U; i < precision - 1U; ++i)
    {
        float y = -radius + heightStep * static_cast<float>(i);
        float cos = fabsf(y) / radius;
        float sin = sqrtf(1.0f - cos * cos);
        std::vector<glm::vec3> circle = createHorizontalCircle(precision, radius * sin, 0.0f, y, 0.0f);
        if (circle.empty())
        {
            return {};
        }
        float texY = i % 2 == 0 ? 0.0f : 1.0f;
        for (unsigned int j = 0; j < circle.size(); ++j)
        {
            float texX = j % 2 == 0 ? 0.0f : 1.0f;
            vertices.emplace_back(circle[j]);
            texCoords.emplace_back(texX, texY);
        }
    }
    unsigned int bottomIndex = static_cast<unsigned int>(vertices.size()), topIndex = bottomIndex + 1U;
    vertices.emplace_back(0.0f, -radius, 0.0f);
    vertices.emplace_back(0.0f, radius, 0.0f);
    texCoords.emplace_back(0.5f, 1.0f);
    texCoords.emplace_back(0.5f, 0.0f);

    unsigned int heightSegments = precision - 3U;
    unsigned int topCircleBegin = precision * heightSegments;
    for (unsigned int i = 0; i < precision; ++i)
    {
        unsigned int nextIndex = (i + 1U) % precision;
        indices.insert(indices.end(), { nextIndex,i,bottomIndex,i + topCircleBegin,nextIndex + topCircleBegin,topIndex });
    }
    for (unsigned int i = 0; i < heightSegments; ++i)
    {
        unsigned int currStart = i * precision;
        for (unsigned int j = 0; j < precision; ++j)
        {
            unsigned int currIndex = currStart + j;
            unsigned int nextIndex = currStart + (j + 1U) % precision;
            unsigned int upper = currIndex + precision;
            unsigned int upperNext = nextIndex + precision;
            indices.insert(indices.end(), { currIndex,nextIndex,upperNext,currIndex,upperNext,upper });
        }
    }
    return std::make_shared<PrimitiveData>(vertices, texCoords, indices);
}

std::shared_ptr<shadow::PrimitiveData> shadow::Primitives::torus(unsigned int precision)
{
    return torus(precision, 0.5f, 1.0f);
}

std::shared_ptr<shadow::PrimitiveData> shadow::Primitives::torus(unsigned int precision, float innerRadius, float outerRadius)
{
    const float midDistance = (innerRadius + outerRadius) * 0.5f;
    const float midRadius = (outerRadius - innerRadius) * 0.5f;
    float angle = 0.0f;
    const float angleStep = FPI * 2.0f / static_cast<float>(precision);
    std::vector<glm::vec3> horizontalCircle = createHorizontalCircle(precision, midDistance, 0.0f, 0.0f, 0.0f);
    if (horizontalCircle.empty())
    {
        return {};
    }
    std::vector<glm::vec3> vertices{};
    std::vector<glm::vec2> texCoords{};
    std::vector<GLuint> indices{};
    for (unsigned int i = 0; i < precision; ++i)
    {
        glm::vec3& point = horizontalCircle[i];
        std::vector<glm::vec3> circle = createVerticalCircle(precision, midRadius, angle, point[0], 0.0f, point[2]);
        if (circle.empty())
        {
            return {};
        }
        angle += angleStep;
        float texX = i % 2 == 0 ? 0.0f : 1.0f;
        for (unsigned int j = 0; j < circle.size(); ++j)
        {
            float texY = j % 2 == 0 ? 0.0f : 1.0f;
            vertices.push_back(circle[j]);
            texCoords.emplace_back(texX, texY);
        }
    }
    for (unsigned int i = 0; i < precision; ++i)
    {
        unsigned int currStart = i * precision;
        unsigned int nextStart = ((i + 1U) % precision) * precision;
        for (unsigned int j = 0; j < precision; ++j)
        {
            unsigned int currIndex = currStart + j;
            unsigned int nextIndex = (j + 1U) % precision;
            unsigned int nexSegCurrIndex = nextStart + j;
            unsigned int nexSegNextIndex = nextStart + nextIndex;
            nextIndex += currStart;
            indices.insert(indices.end(), { nextIndex,currIndex,nexSegNextIndex,nexSegNextIndex,currIndex,nexSegCurrIndex });
        }
    }
    return std::make_shared<PrimitiveData>(vertices, texCoords, indices);
}

std::vector<glm::vec3> shadow::Primitives::createHorizontalCircle(unsigned int precision, float radius, float x, float y, float z)
{
    std::vector<glm::vec3> result{};
    if (precision < 3)
    {
        SHADOW_ERROR("Precision {} < 3!", precision);
        return {};
    }
    float angle = 0.0f;
    const float angleStep = FPI * 2.0f / static_cast<float>(precision);
    for (unsigned int i = 0; i < precision; ++i)
    {
        result.emplace_back(cosf(angle) * radius + x, y, -sinf(angle) * radius + z);
        angle += angleStep;
    }
    return result;
}

std::vector<glm::vec3> shadow::Primitives::createVerticalCircle(unsigned int precision, float radius, float horizontalAngle, float x, float y, float z)
{
    std::vector<glm::vec3> result{};
    if (precision < 3)
    {
        SHADOW_ERROR("Precision {} < 3!", precision);
        return {};
    }
    float angle = 0.0f;
    const float angleStep = FPI * 2.0f / static_cast<float>(precision);
    const float horizontalSin = sinf(horizontalAngle);
    const float horizontalCos = cosf(horizontalAngle);
    for (unsigned int i = 0; i < precision; ++i)
    {
        const float xAdd = cosf(angle) * radius;
        result.emplace_back(xAdd * horizontalCos + x, sinf(angle) * radius + y, -xAdd * horizontalSin + z);
        angle += angleStep;
    }
    return result;
}
